#include <cuzn/journeys/BuildJourney.h>
#include <m2/ui/State.h>
#include <cuzn/Detail.h>
#include <cuzn/detail/Build.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/Coal.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include "cuzn/object/Factory.h"
#include "cuzn/detail/Network.h"
#include <cuzn/object/GameStateTracker.h>

using namespace m2;
using namespace m2::ui;
using namespace m2g;
using namespace m2g::pb;

namespace {
	std::set<IndustryLocation> buildable_industry_locations_in_network_with_card(m2::Character& player, Card card) {
		if (not is_card(card)) {
			throw M2_ERROR("Item is not a card");
		}
		if (not player_has_card(player, card)) {
			throw M2_ERROR("Player does not own the given card");
		}

		// Gather industry locations in player's network
		std::set<IndustryLocation> industry_locations_in_network;
		if (auto cities_in_network = get_cities_in_network(player); cities_in_network.empty() || card == WILD_LOCATION_CARD) {
			// If there are no locations in network, or the wild location card is selected, all locations are considered in-network
			industry_locations_in_network = all_industry_locations();
		} else {
			if (M2G_PROXY.is_canal_era()) {
				// If canal era, remove the cities where there is already an industry of player
				for (const auto& built_factory_location : player_built_factory_locations(player)) {
					cities_in_network.erase(city_of_location(built_factory_location));
				}
			}

			for (const auto& city : cities_in_network) {
				// Insert locations in cities
				auto locs_in_city = industry_locations_in_city(city);
				industry_locations_in_network.insert(locs_in_city.begin(), locs_in_city.end());
			}
		}

		// Filter industry_locations_in_network by card
		if (card == WILD_LOCATION_CARD || card == WILD_INDUSTRY_CARD) {
			// No filtering
			return industry_locations_in_network;
		} else if (M2_GAME.named_items[card].category() == ITEM_CATEGORY_INDUSTRY_CARD) {
			// Filter by industry
			auto filtered_locations = industry_locations_in_network | std::views::filter([card](IndustryLocation location) {
				auto industries = industries_on_location(location);
				return std::find(industries.begin(), industries.end(), card) != industries.end();
			});
			return {filtered_locations.begin(), filtered_locations.end()};
		} else if (M2_GAME.named_items[card].category() == ITEM_CATEGORY_CITY_CARD) {
			// Filter by city
			auto filtered_locations = industry_locations_in_network | std::views::filter([card](IndustryLocation location) {
				return city_of_location(location) == card;
			});
			return {filtered_locations.begin(), filtered_locations.end()};
		} else {
			throw M2_ERROR("Invalid card category");
		}
	}
}

m2::void_expected can_player_attempt_to_build(m2::Character& player) {
	if (player_card_count(player) < 1) {
		return m2::make_unexpected("Build action requires a card");
	}

	if (player_tile_count(player) < 1) {
		return m2::make_unexpected("Build action requires an industry tile");
	}

	return {};
}

BuildJourney::BuildJourney() : m2::FsmBase<BuildJourneyStep, PositionOrCancelSignal>() {
	DEBUG_FN();
	init(BuildJourneyStep::INITIAL_STEP);
}

std::optional<BuildJourneyStep> BuildJourney::handle_signal(const PositionOrCancelSignal& s) {
	switch (state()) {
		case BuildJourneyStep::INITIAL_STEP:
			switch (s.type()) {
				case m2::FsmSignalType::EnterState: return handle_initial_enter_signal();
				case m2::FsmSignalType::ExitState: return std::nullopt;
				default: throw M2_ERROR("Unexpected signal");
			}
		case BuildJourneyStep::EXPECT_LOCATION:
			switch (s.type()) {
				case m2::FsmSignalType::EnterState: return handle_location_enter_signal();
				case m2::FsmSignalType::Custom: {
					if (auto world_position = s.world_position(); world_position) {
						return handle_location_mouse_click_signal(*world_position);
					} else if (s.cancel()) {
						return handle_location_cancel_signal();
					}
					throw M2_ERROR("Unexpected signal");
				}
				case m2::FsmSignalType::ExitState: return handle_location_exit_signal();
				default: throw M2_ERROR("Unexpected signal");
			}
		case BuildJourneyStep::EXPECT_RESOURCE_SOURCE:
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
			return std::nullopt;
		case BuildJourneyStep::EXPECT_CONFIRMATION:
			switch (s.type()) {
				case FsmSignalType::EnterState: return handle_confirmation_enter_signal();
				case FsmSignalType::ExitState: return std::nullopt;
				default: throw M2_ERROR("Unexpected signal");
			}
	}
}

std::optional<BuildJourneyStep> BuildJourney::handle_initial_enter_signal() {
	if (auto selected_card = ask_for_card_selection(); selected_card) {
		_selected_card = *selected_card;
		return BuildJourneyStep::EXPECT_LOCATION;
	} else {
		M2_DEFER(m2g::Proxy::user_journey_deleter);
		return std::nullopt;
	}
}

std::optional<BuildJourneyStep> BuildJourney::handle_location_enter_signal() {
	LOG_DEBUG("Expecting build location...");
	M2_LEVEL.disable_hud();
	M2_LEVEL.display_message("Pick location");
	M2_LEVEL.add_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX, RectF{0.775f, 0.1f, 0.15f, 0.1f}, &journey_cancel_button);
	// Dim places outside the player's network
	auto buildable_locs = buildable_industry_locations_in_network_with_card(M2_PLAYER.character(), _selected_card);
	M2_GAME.enable_dimming_with_exceptions(M2G_PROXY.object_ids_of_industry_location_bg_tiles(buildable_locs));
	return std::nullopt;
}

std::optional<BuildJourneyStep> BuildJourney::handle_location_mouse_click_signal(const m2::VecF& world_position) {
	LOG_DEBUG("Received mouse click", world_position);
	if (auto selected_loc = industry_location_on_position(world_position)) {
		LOG_INFO("Clicked on", m2g::pb::SpriteType_Name(*selected_loc));

		// Check if there's a need to make an industry selection based on the card and the sprite
		if (auto buildable_inds = buildable_industries(_selected_card, *selected_loc); buildable_inds.empty()) {
			M2_LEVEL.display_message("Selected position cannot be built with the selected card", 10.0f);
			return std::nullopt;
		} else if (buildable_inds.size() == 2) {
			if (auto selected_industry = ask_for_industry_selection(buildable_inds[0], buildable_inds[1]); selected_industry) {
				_selected_industry = *selected_industry;
			} else {
				M2_DEFER(m2g::Proxy::user_journey_deleter);
				return std::nullopt;
			}
		} else if (buildable_inds.size() == 1) {
			_selected_industry = buildable_inds[0];
		} else {
			throw M2_ERROR("Implementation error, more than 2 selectable industries in one location");
		}
		_selected_location = *selected_loc;

		// Check if the player has a factory to build
		auto tile_type = get_next_buildable_factory(M2_PLAYER.character(), industry_tile_category_of_industry(_selected_industry));
		if (not tile_type) {
			M2_LEVEL.display_message("Player doesn't have an industry tile of appropriate type");
			M2_DEFER(m2g::Proxy::user_journey_deleter);
			return std::nullopt;
		}
		_industry_tile = *tile_type;

		// Create empty entries in resource_sources for every required resource
		_resource_sources.insert(_resource_sources.end(),
			iround(M2_GAME.get_named_item(*tile_type).get_attribute(COAL_COST)),
			std::make_pair(COAL_CUBE_COUNT, NO_SPRITE));
		_resource_sources.insert(_resource_sources.end(),
			iround(M2_GAME.get_named_item(*tile_type).get_attribute(IRON_COST)),
			std::make_pair(IRON_CUBE_COUNT, NO_SPRITE));
		return _resource_sources.empty() ? BuildJourneyStep::EXPECT_CONFIRMATION : BuildJourneyStep::EXPECT_RESOURCE_SOURCE;
	}
	LOG_DEBUG("Selected position was not on an industry");
	return std::nullopt;
}

std::optional<BuildJourneyStep> BuildJourney::handle_location_cancel_signal() {
	LOG_INFO("Cancelling Build action...");
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}

std::optional<BuildJourneyStep> BuildJourney::handle_location_exit_signal() {
	M2_LEVEL.enable_hud();
	M2_LEVEL.remove_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX);
	// Disable dimming in case it was enabled
	M2_GAME.disable_dimming_with_exceptions();
	return std::nullopt;
}

std::optional<BuildJourneyStep> BuildJourney::handle_resource_enter_signal() {
	// Check if there's an unspecified resource left
	if (auto unspecified_resource = get_next_unspecified_resource(); unspecified_resource != _resource_sources.end()) {
		auto index = std::distance(_resource_sources.begin(), unspecified_resource) + 1;
		if (unspecified_resource->first == COAL_CUBE_COUNT) {
			auto selected_city = city_of_location(_selected_location);
			auto closest_mines_with_coal = find_closest_connected_coal_mines_with_coal(selected_city);
			if (closest_mines_with_coal.empty()) {
				// No reachable coal mines with coal, check the coal market
				if (auto coal_market_city = find_connected_coal_market_with_coal(selected_city)) {
					// If no reachable coal has left on the map, all the remaining coal must come from the market
					auto remaining_unspecified_coal_count = std::count(_resource_sources.begin(), _resource_sources.end(),
						std::make_pair(COAL_CUBE_COUNT, NO_SPRITE));
					// Calculate the cost of buying coal
					auto cost_of_buying = market_coal_cost(m2::I(remaining_unspecified_coal_count));
					// Merchant location
					auto merchant_location = merchant_locations_of_merchant_city(*coal_market_city)[0];
					// Get a game drawing centered at the merchant location
					auto background = M2_GAME.draw_game_to_texture(M2G_PROXY.merchant_positions[merchant_location].first);
					LOG_DEBUG("Asking player if they want to buy coal from the market...");
					if (ask_for_confirmation_bottom("Buy " + std::to_string(remaining_unspecified_coal_count) + " coal from market for £" + std::to_string(cost_of_buying) + "?", "Yes", "No", std::move(background))) {
						LOG_DEBUG("Player agreed");
						// Specify resource sources
						std::replace(_resource_sources.begin(), _resource_sources.end(),
							std::make_pair(COAL_CUBE_COUNT, NO_SPRITE), std::make_pair(COAL_CUBE_COUNT, merchant_location));
						// Re-enter resource selection
						return BuildJourneyStep::EXPECT_RESOURCE_SOURCE;
					} else {
						LOG_INFO("Player declined, cancelling Build action...");
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
					// Specify resource source
					unspecified_resource->second = closest_mines_with_coal[0];
					// Re-enter resource selection
					return BuildJourneyStep::EXPECT_RESOURCE_SOURCE;
				} else {
					LOG_INFO("Player declined, cancelling Build action...");
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
				M2_LEVEL.add_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX, RectF{0.775f, 0.1f, 0.15f, 0.1f}, &journey_cancel_button);
				M2_LEVEL.display_message(std::to_string(index) +  "/" + std::to_string(_resource_sources.size()) + ": Pick a coal source");
			}
		} else if (unspecified_resource->first == IRON_CUBE_COUNT) {
			LOG_DEBUG("Asking player to pick an iron source...");
			M2_LEVEL.disable_hud();
			M2_LEVEL.add_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX, RectF{0.775f, 0.1f, 0.15f, 0.1f}, &journey_cancel_button);
			M2_LEVEL.display_message(std::to_string(index) +  "/" + std::to_string(_resource_sources.size()) + ": Pick an iron source");
		} else {
			throw M2_ERROR("Unexpected resource in resource list");
		}
		return std::nullopt;
	} else {
		return BuildJourneyStep::EXPECT_CONFIRMATION;
	}
}

std::optional<BuildJourneyStep> BuildJourney::handle_resource_mouse_click_signal(const m2::VecF& world_position) {
	LOG_DEBUG("Received mouse click", world_position);

	auto unspecified_resource = get_next_unspecified_resource();
	if (auto industry_loc = industry_location_on_position(world_position)) {
		LOG_DEBUG("Industry location", m2g::pb::SpriteType_Name(*industry_loc));
		// Check if location has a built factory
		if (auto* factory = find_factory_at_location(*industry_loc)) {
			// Check if factory has the required resource
			if (m2::god_mode || m2::is_less_or_equal(1.0f, factory->character().get_resource(unspecified_resource->first), 0.001f)) {
				// Check if the factory is connected
				if (m2::god_mode || unspecified_resource->first == IRON_CUBE_COUNT || is_industry_city_connected_to_location(city_of_location(_selected_location), *industry_loc)) {
					// Deduct resource
					factory->character().remove_resource(unspecified_resource->first, 1.0f);
					// Specify resource source
					unspecified_resource->second = *industry_loc;
					// Reserve resource
					_reserved_resources.emplace_back(factory, unspecified_resource->first);
					// Re-enter resource selection
					return BuildJourneyStep::EXPECT_RESOURCE_SOURCE;
				} else {
					LOG_DEBUG("There's no connection between selected industries");
				}
			} else {
				LOG_DEBUG("Industry doesn't have the required resource");
			}
		} else {
			LOG_DEBUG("Industry is not built");
		}
	} else if (auto merchant_loc = merchant_location_on_position(world_position)) {
		LOG_DEBUG("Merchant location", m2g::pb::SpriteType_Name(*merchant_loc));
		// Check if the merchant is connected
		if (m2::god_mode || unspecified_resource->first == IRON_CUBE_COUNT || is_industry_city_connected_to_location(city_of_location(_selected_location), *merchant_loc)) {
			// Specify resource source
			unspecified_resource->second = *merchant_loc;
			// Re-enter resource selection
			return BuildJourneyStep::EXPECT_RESOURCE_SOURCE;
		} else {
			LOG_DEBUG("There's no connection between selected industry and the merchant");
		}
	}
	return std::nullopt;
}

std::optional<BuildJourneyStep> BuildJourney::handle_resource_cancel_signal() {
	LOG_INFO("Cancelling Build action...");
	// Return the reserved resources
	for (auto [factory, resource_type] : _reserved_resources) {
		factory->character().add_resource(resource_type, 1.0f);
	}
	_reserved_resources.clear();
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}

std::optional<BuildJourneyStep> BuildJourney::handle_resource_exit_signal() {
	M2_LEVEL.enable_hud();
	M2_LEVEL.remove_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX);
	// Disable dimming in case it was enabled
	M2_GAME.disable_dimming_with_exceptions();
	return std::nullopt;
}

std::optional<BuildJourneyStep> BuildJourney::handle_confirmation_enter_signal() {
	LOG_INFO("Asking for confirmation...");
	auto card_name = M2_GAME.get_named_item(_selected_card).in_game_name();
	auto city_name = M2_GAME.get_named_item(city_of_location(_selected_location)).in_game_name();
	auto industry_name = M2_GAME.get_named_item(_selected_industry).in_game_name();
	if (ask_for_confirmation("Build " + industry_name + " in " + city_name, "using " + card_name + " card?", "OK", "Cancel")) {
		LOG_INFO("Build action confirmed");
		M2_LEVEL.display_message("Building location...");

		m2g::pb::ClientCommand cc;
		cc.mutable_build_action()->set_card(_selected_card);
		cc.mutable_build_action()->set_industry_location(_selected_location);
		cc.mutable_build_action()->set_industry_tile(_industry_tile);
		for (const auto& resource_source : _resource_sources) {
			if (resource_source.first == COAL_CUBE_COUNT) {
				cc.mutable_build_action()->add_coal_sources(resource_source.second);
			} else if (resource_source.first == IRON_CUBE_COUNT) {
				cc.mutable_build_action()->add_iron_sources(resource_source.second);
			} else {
				throw M2_ERROR("Unexpected resource type");
			}
		}
		M2_GAME.client_thread().queue_client_command(cc);
	} else {
		LOG_INFO("Cancelling Build action...");
		// Return the reserved resources
		for (auto [factory, resource_type] : _reserved_resources) {
			factory->character().add_resource(resource_type, 1.0f);
		}
		_reserved_resources.clear();
	}
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}

decltype(BuildJourney::_resource_sources)::iterator BuildJourney::get_next_unspecified_resource() {
	return std::find_if(_resource_sources.begin(), _resource_sources.end(), [](const auto& r) {
		return r.second == NO_SPRITE;
	});
}
