#include <cuzn/journeys/NetworkJourney.h>
#include <cuzn/Detail.h>
#include <cuzn/detail/Network.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Cards.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/ConsumingCoal.h>
#include <cuzn/ConsumingBeer.h>
#include <cuzn/object/Road.h>
#include <cuzn/object/Factory.h>
#include <cuzn/object/GameStateTracker.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2g/Proxy.h>

using namespace m2;
using namespace m2::ui;
using namespace m2g;
using namespace m2g::pb;

namespace {
	int road_cost(bool build_double) {
		if (M2G_PROXY.is_canal_era()) {
			return 3;
		} else {
			return build_double ? 15 : 5;
		}
	}

	std::set<Connection> buildable_connections_in_network(m2::Character& player, Connection provisional_extra_connection = {}) {
		// Gather connections in player's network
		std::set<Connection> connections_in_network = get_connections_in_network(player, provisional_extra_connection);
		if (connections_in_network.empty()) {
			connections_in_network = M2G_PROXY.is_canal_era() ? all_canals() : all_railroads();
		}
		// Filter built locations
		for (auto it = connections_in_network.begin(); it != connections_in_network.end(); ) {
			if (find_road_at_location(*it)) {
				it = connections_in_network.erase(it);
			} else {
				++it;
			}
		}
		return connections_in_network;
	}
}

m2::void_expected can_player_attempt_to_network(m2::Character& player) {
	if (player_card_count(player) < 1) {
		return m2::make_unexpected("Network action requires a card");
	}

	if (player_available_road_count(player) < 1) {
		return m2::make_unexpected("Network action requires a link tile");
	}

	return {};
}

NetworkJourney::NetworkJourney() : FsmBase() {
	DEBUG_FN();
	init(NetworkJourneyStep::INITIAL_STEP);
}

NetworkJourney::~NetworkJourney() {
	for (auto& source : _resource_sources) {
		if (source.reserved_object) {
			source.reserved_object->character().add_resource(source.resource_type, 1.0f);
			source.reserved_object = nullptr;
		}
	}
	if (_decoy_road_1) {
		m2::create_object_deleter(_decoy_road_1)();
		_decoy_road_1 = 0;
	}
	if (_decoy_road_2) {
		m2::create_object_deleter(_decoy_road_2)();
		_decoy_road_2 = 0;
	}
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_signal(const PositionOrCancelSignal& s) {
	switch (state()) {
		case NetworkJourneyStep::INITIAL_STEP:
			switch (s.type()) {
				case FsmSignalType::EnterState: return handle_initial_enter_signal();
				case FsmSignalType::ExitState: return std::nullopt;
				default: throw M2_ERROR("Unexpected signal");
			}
		case NetworkJourneyStep::EXPECT_LOCATION:
			switch (s.type()) {
				case FsmSignalType::EnterState: return handle_location_enter_signal();
				case FsmSignalType::Custom: {
					if (auto world_position = s.world_position(); world_position) {
						return handle_location_mouse_click_signal(*world_position);
					} else if (s.cancel()) {
						return handle_location_cancel_signal();
					}
					throw M2_ERROR("Unexpected signal");
				}
				case FsmSignalType::ExitState: return handle_location_exit_signal();
				default: throw M2_ERROR("Unexpected signal");
			}
		case NetworkJourneyStep::EXPECT_RESOURCE_SOURCE:
			switch (s.type()) {
				case FsmSignalType::EnterState: return handle_resource_enter_signal();
				case FsmSignalType::Custom: {
					if (auto world_position = s.world_position(); world_position) {
						return handle_resource_mouse_click_signal(*world_position);
					} else if (s.cancel()) {
						return handle_resource_cancel_signal();
					}
					throw M2_ERROR("Unexpected signal");
				}
				case FsmSignalType::ExitState: return handle_resource_exit_signal();
				default: throw M2_ERROR("Unexpected signal");
			}
		case NetworkJourneyStep::EXPECT_CONFIRMATION:
			switch (s.type()) {
				case FsmSignalType::EnterState: return handle_confirmation_enter_signal();
				case FsmSignalType::ExitState: return std::nullopt;
				default: throw M2_ERROR("Unexpected signal");
			}
	}
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_initial_enter_signal() {
	// Ask if double railroads should be built
	if (M2G_PROXY.is_railroad_era() && 1 < M2_PLAYER.character().count_item(m2g::pb::ROAD_TILE)) {
		_build_double_railroads = ask_for_confirmation("Build double railroads?", "", "Yes", "No");
	}

	if (auto selected_card = ask_for_card_selection()) {
		_selected_card = *selected_card;
		return NetworkJourneyStep::EXPECT_LOCATION;
	} else {
		M2_DEFER(m2g::Proxy::user_journey_deleter);
		return std::nullopt;
	}
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_location_enter_signal() {
	LOG_DEBUG("Expecting connection...");
	M2_LEVEL.disable_hud();
	M2_LEVEL.display_message("Pick connection", -1.0f);
	_cancel_button_panel = M2_LEVEL.add_custom_nonblocking_ui_panel(Panel{&journey_cancel_button, RectF{0.775f, 0.1f, 0.15f, 0.1f}});
	// Dim places outside the player's network
	_buildable_connections = buildable_connections_in_network(M2_PLAYER.character());
	M2_GAME.enable_dimming_with_exceptions(M2G_PROXY.object_ids_of_connection_bg_tiles(_buildable_connections));
	return std::nullopt;
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_location_mouse_click_signal(const m2::VecF& world_position) {
	LOG_DEBUG("Received mouse click", world_position);
	if (auto selected_loc = connection_on_position(world_position); selected_loc && _buildable_connections.contains(*selected_loc)) {
		LOG_INFO("Clicked on", m2g::pb::SpriteType_Name(*selected_loc));

		if (!_selected_connection_1) {
			_selected_connection_1 = *selected_loc;
			// Update buildable connections with the new selection
			_buildable_connections = buildable_connections_in_network(M2_PLAYER.character(), _selected_connection_1);
			M2_GAME.enable_dimming_with_exceptions(M2G_PROXY.object_ids_of_connection_bg_tiles(_buildable_connections));
		} else if (_build_double_railroads && !_selected_connection_2 && _selected_connection_1 != *selected_loc) {
			_selected_connection_2 = *selected_loc;
		}

		// If selection done
		if ((not _build_double_railroads && _selected_connection_1) || _selected_connection_2) {
			_resource_sources = required_resources_for_network();
			return _resource_sources.empty() ? NetworkJourneyStep::EXPECT_CONFIRMATION : NetworkJourneyStep::EXPECT_RESOURCE_SOURCE;
		}
	}
	LOG_DEBUG("Selected position was not on a buildable connection");
	return std::nullopt;
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_location_cancel_signal() {
	LOG_INFO("Cancelling Network action...");
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_location_exit_signal() {
	M2_LEVEL.enable_hud();
	M2_LEVEL.remove_message();
	if (_cancel_button_panel) {
		M2_LEVEL.remove_custom_nonblocking_ui_panel(*_cancel_button_panel);
		_cancel_button_panel.reset();
	}
	// Disable dimming in case it was enabled
	M2_GAME.disable_dimming_with_exceptions();
	return std::nullopt;
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_resource_enter_signal() {
	// Check if there's an unspecified resource left
	if (auto unspecified_resource = get_next_unspecified_resource(); unspecified_resource != _resource_sources.end()) {
		// Create the decoy connections if necessary
		if (unspecified_resource->connection == _selected_connection_1 && not _decoy_road_1) {
			// Create the first decoy
			auto it = m2::create_object(position_of_connection(_selected_connection_1), m2g::pb::ROAD, M2_PLAYER.id());
			init_road(*it, _selected_connection_1);
			_decoy_road_1 = it->id();
		}
		if (unspecified_resource->connection == _selected_connection_2 && not _decoy_road_2) {
			// Create the second decoy
			auto it = m2::create_object(position_of_connection(_selected_connection_2), m2g::pb::ROAD, M2_PLAYER.id());
			init_road(*it, _selected_connection_2);
			_decoy_road_2 = it->id();
		}

		auto major_cities = major_cities_from_connection(unspecified_resource->connection);
		if (unspecified_resource->resource_type == COAL_CUBE_COUNT) {
			if (auto closest_mines_with_coal = find_closest_connected_coal_mines_with_coal(major_cities[0], major_cities[1]); closest_mines_with_coal.empty()) {
				// No reachable coal mines with coal, check the coal market
				if (auto coal_market_city = find_connected_coal_market(major_cities[0], major_cities[1])) {
					// Merchant location
					auto merchant_location = merchant_locations_of_merchant_city(*coal_market_city)[0];
					// Get a game drawing centered at the merchant location
					auto background = M2_GAME.draw_game_to_texture(std::get<m2::VecF>(M2G_PROXY.merchant_positions[merchant_location]));
					LOG_DEBUG("Asking player if they want to buy coal from the market...");
					if (ask_for_confirmation_bottom("Buy 1 coal from market for £" + std::to_string(M2G_PROXY.market_coal_cost(1)) + "?", "Yes", "No", std::move(background))) {
						LOG_DEBUG("Player agreed");
						// Specify resource source
						unspecified_resource->source = merchant_location;
						// Re-enter resource selection
						return NetworkJourneyStep::EXPECT_RESOURCE_SOURCE;
					} else {
						LOG_INFO("Player declined, cancelling Network action...");
						M2_DEFER(m2g::Proxy::user_journey_deleter);
					}
				} else {
					M2_LEVEL.display_message("Coal required but none available in network");
					M2_DEFER(m2g::Proxy::user_journey_deleter);
				}
			} else if (closest_mines_with_coal.size() == 1) {
				// Only one viable coal mine with coal is in the vicinity, confirm with the player.
				// Get a game drawing centered at the industry location
				auto background = M2_GAME.draw_game_to_texture(std::get<m2::VecF>(M2G_PROXY.industry_positions[closest_mines_with_coal[0]]));
				LOG_DEBUG("Asking player if they want to buy coal from the closest mine...");
				if (ask_for_confirmation_bottom("Buy coal from shown mine for free?", "Yes", "No", std::move(background))) {
					LOG_DEBUG("Player agreed");
					// Reserve resource
					auto* factory = find_factory_at_location(closest_mines_with_coal[0]);
					factory->character().remove_resource(COAL_CUBE_COUNT, 1.0f);
					unspecified_resource->reserved_object = factory;
					// Specify resource source
					unspecified_resource->source = closest_mines_with_coal[0];
					// Re-enter resource selection
					return NetworkJourneyStep::EXPECT_RESOURCE_SOURCE;
				} else {
					LOG_INFO("Player declined, cancelling Network action...");
					M2_DEFER(m2g::Proxy::user_journey_deleter);
				}
			} else {
				// Look-up ObjectIDs of closest coal mines
				std::set<ObjectId> coal_mine_object_ids;
				std::transform(closest_mines_with_coal.begin(), closest_mines_with_coal.end(),
					std::inserter(coal_mine_object_ids, coal_mine_object_ids.begin()),
					[](IndustryLocation loc) { return find_factory_at_location(loc)->id(); });
				// Enable dimming except the coal mines
				M2_GAME.enable_dimming_with_exceptions(coal_mine_object_ids);
				LOG_DEBUG("Asking player to pick a coal source...");

				M2_LEVEL.disable_hud();
				_cancel_button_panel = M2_LEVEL.add_custom_nonblocking_ui_panel(Panel{&journey_cancel_button, RectF{0.775f, 0.1f, 0.15f, 0.1f}});
				M2_LEVEL.display_message("Pick a coal source");
			}
		} else if (unspecified_resource->resource_type == BEER_BARREL_COUNT) {
			if (auto beer_sources = find_breweries_with_beer(M2_PLAYER.character(), major_cities[0], std::nullopt, major_cities[1]); beer_sources.empty()) {
				M2_LEVEL.display_message("Beer required but none available in network");
				M2_DEFER(m2g::Proxy::user_journey_deleter);
			} else if (beer_sources.size() == 1) {
				auto industry_location = *beer_sources.begin(); // While networking, beer only comes from industries.
				// Only one viable beer industry with beer is in the vicinity, confirm with the player.
				// Get a game drawing centered at the industry location
				auto background = M2_GAME.draw_game_to_texture(std::get<m2::VecF>(M2G_PROXY.industry_positions[industry_location]));
				LOG_DEBUG("Asking player if they want to buy beer from the closest industry...");
				if (ask_for_confirmation_bottom("Buy beer from shown industry for free?", "Yes", "No", std::move(background))) {
					LOG_DEBUG("Player agreed");
					// Reserve resource
					auto* factory = find_factory_at_location(industry_location);
					factory->character().remove_resource(BEER_BARREL_COUNT, 1.0f);
					unspecified_resource->reserved_object = factory;
					// Specify resource source
					unspecified_resource->source = industry_location;
					// Re-enter resource selection
					return NetworkJourneyStep::EXPECT_RESOURCE_SOURCE;
				} else {
					LOG_INFO("Player declined, cancelling Build action...");
					M2_DEFER(m2g::Proxy::user_journey_deleter);
				}
			} else {
				std::set<ObjectId> brewery_object_ids; // Look-up ObjectIDs of closest breweries
				std::transform(beer_sources.begin(), beer_sources.end(),
					std::inserter(brewery_object_ids, brewery_object_ids.begin()),
					[](IndustryLocation loc) { return find_factory_at_location(loc)->id(); });
				// Enable dimming except the coal mines
				M2_GAME.enable_dimming_with_exceptions(brewery_object_ids);
				LOG_DEBUG("Asking player to pick a beer source...");

				M2_LEVEL.disable_hud();
				_cancel_button_panel = M2_LEVEL.add_custom_nonblocking_ui_panel(Panel{&journey_cancel_button, RectF{0.775f, 0.1f, 0.15f, 0.1f}});
				M2_LEVEL.display_message("Pick a beer source");
			}
		} else {
			throw M2_ERROR("Unexpected resource in resource list");
		}
		return std::nullopt;
	} else {
		return NetworkJourneyStep::EXPECT_CONFIRMATION;
	}
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_resource_mouse_click_signal(const m2::VecF& world_position) {
	LOG_DEBUG("Received mouse click", world_position);

	auto unspecified_resource = get_next_unspecified_resource();
	if (auto industry_location = industry_location_on_position(world_position)) {
		LOG_DEBUG("Industry location", m2g::pb::SpriteType_Name(*industry_location));
		// Check if location has a built factory
		if (auto* factory = find_factory_at_location(*industry_location)) {
			// Check if the location is one of the dimming exceptions
			if (M2_GAME.dimming_exceptions()->contains(factory->id())) {
				// Reserve resource
				factory->character().remove_resource(unspecified_resource->resource_type, 1.0f);
				unspecified_resource->reserved_object = factory;
				unspecified_resource->source = *industry_location;
				// Re-enter resource selection
				return NetworkJourneyStep::EXPECT_RESOURCE_SOURCE;
			}
		}
	}
	return std::nullopt;
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_resource_cancel_signal() {
	LOG_INFO("Cancelling Network action...");
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_resource_exit_signal() {
	M2_LEVEL.enable_hud();
	if (_cancel_button_panel) {
		M2_LEVEL.remove_custom_nonblocking_ui_panel(*_cancel_button_panel);
		_cancel_button_panel.reset();
	}
	// Disable dimming in case it was enabled
	M2_GAME.disable_dimming_with_exceptions();
	return std::nullopt;
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_confirmation_enter_signal() {
	LOG_INFO("Asking for confirmation...");
	// TODO
	if (ask_for_confirmation("Are you sure?", "", "OK", "Cancel")) {
		LOG_INFO("Network action confirmed");

		m2g::pb::ClientCommand cc;
		cc.mutable_network_action()->set_card(_selected_card);
		cc.mutable_network_action()->set_connection_1(_selected_connection_1);
		if (_build_double_railroads) {
			cc.mutable_network_action()->set_connection_2(_selected_connection_2);
		}
		for (const auto& resource_source : _resource_sources) {
			if (resource_source.resource_type == COAL_CUBE_COUNT) {
				cc.mutable_network_action()->add_coal_sources(resource_source.source);
			} else if (resource_source.resource_type == BEER_BARREL_COUNT) {
				cc.mutable_network_action()->set_beer_source(resource_source.source);
			} else {
				throw M2_ERROR("Unexpected resource type");
			}
		}
		M2_GAME.queue_client_command(cc);
	} else {
		LOG_INFO("Cancelling Network action...");
	}
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}

std::vector<NetworkJourney::ResourceSource> NetworkJourney::required_resources_for_network() {
	if (not _selected_connection_1) {
		throw M2_ERROR("No connection is selected");
	}

	if (M2G_PROXY.is_canal_era()) {
		return {};
	} else if (!_build_double_railroads) {
		return {{_selected_connection_1, COAL_CUBE_COUNT, NO_SPRITE}};
	} else {
		if (not _selected_connection_2) {
			throw M2_ERROR("No second connection is selected");
		}

		return {{_selected_connection_1, COAL_CUBE_COUNT, NO_SPRITE}, {_selected_connection_2, COAL_CUBE_COUNT, NO_SPRITE}, {_selected_connection_2, BEER_BARREL_COUNT, NO_SPRITE}};
	}
}

decltype(NetworkJourney::_resource_sources)::iterator NetworkJourney::get_next_unspecified_resource() {
	return std::find_if(_resource_sources.begin(), _resource_sources.end(), [](const auto& r) {
		return r.source == NO_SPRITE;
	});
}

bool can_player_network(m2::Character& player, const m2g::pb::ClientCommand_NetworkAction& network_action) {
	// Check if the prerequisites are met
	if (auto prerequisite = can_player_attempt_to_network(player); not prerequisite) {
		LOG_WARN("Player does not meet network prerequisites", prerequisite.error());
		return false;
	}

	// Check if the player holds the selected card
	if (not is_card(network_action.card())) {
		LOG_WARN("Selected card is not a card");
		return false;
	}
	if (player.find_items(network_action.card()) == player.end_items()) {
		LOG_WARN("Player does not have the selected card");
		return false;
	}

	// Check if the player has network tiles
	if (not is_connection(network_action.connection_1())) {
		LOG_WARN("Selected connection is not a connection");
		return false;
	}
	if (network_action.connection_2()) {
		if (not is_connection(network_action.connection_2())) {
			LOG_WARN("Selected connection is not a connection");
			return false;
		}
		if (network_action.connection_1() == network_action.connection_2()) {
			LOG_WARN("Selected connections are the same");
			return false;
		}
	}
	if (player.count_item(ROAD_TILE) < (network_action.connection_2() ? 2 : 1)) {
		LOG_WARN("Player doesn't have enough road tiles");
		return false;
	}
	// Check if the connections can be built in this era
	if (M2G_PROXY.is_canal_era()) {
		if (not is_canal(network_action.connection_1()) || (network_action.connection_2() && not is_canal(network_action.connection_2()))) {
			LOG_WARN("Selected connection cannot be built in this era");
			return false;
		}
	} else {
		if (not is_railroad(network_action.connection_1()) || (network_action.connection_2() && not is_railroad(network_action.connection_2()))) {
			LOG_WARN("Selected connection cannot be built in this era");
			return false;
		}
	}

	// TODO Check if the connection is available and the player can build them

	// TODO Check if the player can use the selected resources

	// TODO Check if all required resources are provided

	// Calculate cost
	auto coal_from_market = std::ranges::count_if(network_action.coal_sources(), [](const auto& coal_source) {
		return is_merchant_location(static_cast<Location>(coal_source));
	});
	// Check if the player has enough money
	if (m2::iround(player.get_resource(MONEY)) < road_cost(network_action.connection_2()) + M2G_PROXY.market_coal_cost(m2::I(coal_from_market))) {
		LOG_WARN("Player does not have enough money");
		return false;
	}

	return true;
}

std::pair<Card,int> execute_network_action(m2::Character& player, const m2g::pb::ClientCommand_NetworkAction& network_action) {
	// Assume everything is validated

	// Take road tiles from player
	auto road_tile_it = player.find_items(ROAD_TILE);
	player.remove_item(road_tile_it);
	if (network_action.connection_2()) {
		auto road_tile_it_2 = player.find_items(ROAD_TILE);
		player.remove_item(road_tile_it_2);
	}

	// Calculate the cost of building the road
	auto coal_from_market = std::ranges::count_if(network_action.coal_sources(), [](const auto& coal_source) {
		return is_merchant_location(static_cast<Location>(coal_source));
	});
	auto cost = road_cost(network_action.connection_2()) + M2G_PROXY.market_coal_cost(m2::I(coal_from_market));

	// Take resources
	for (const auto& coal_source : network_action.coal_sources()) {
		auto location = static_cast<Location>(coal_source);
		if (is_industry_location(location)) {
			auto* factory = find_factory_at_location(location);
			factory->character().remove_resource(COAL_CUBE_COUNT, 1.0f);
		} else if (is_merchant_location(location)) {
			M2G_PROXY.buy_coal_from_market();
		}
	}
	if (network_action.beer_source()) {
		auto location = static_cast<Location>(network_action.beer_source());
		auto* factory = find_factory_at_location(location);
		factory->character().remove_resource(BEER_BARREL_COUNT, 1.0f);
	}

	// Create the road on the map
	auto it = m2::create_object(position_of_connection(network_action.connection_1()), m2g::pb::ROAD, player.owner_id());
	init_road(*it, network_action.connection_1());
	if (network_action.connection_2()) {
		auto it_2 = m2::create_object(position_of_connection(network_action.connection_2()), m2g::pb::ROAD, player.owner_id());
		init_road(*it_2, network_action.connection_1());
	}

	flip_exhausted_factories();

	return std::make_pair(network_action.card(), cost);
}
