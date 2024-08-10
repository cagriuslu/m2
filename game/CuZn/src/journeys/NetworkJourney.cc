#include <cuzn/journeys/NetworkJourney.h>
#include <cuzn/Detail.h>
#include <cuzn/detail/Network.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/object/Road.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2g/Proxy.h>

using namespace m2;
using namespace m2::ui;
using namespace m2g;
using namespace m2g::pb;

namespace {
	std::vector<std::pair<m2g::pb::ResourceType, m2g::pb::SpriteType>> required_resources_for_network(bool double_railroad) {
		if (M2G_PROXY.is_canal_era()) {
			return {};
		} else if (!double_railroad) {
			return {{COAL_CUBE_COUNT, NO_SPRITE}};
		} else {
			return {{COAL_CUBE_COUNT, NO_SPRITE}, {COAL_CUBE_COUNT, NO_SPRITE}, {BEER_BARREL_COUNT, NO_SPRITE}};
		}
	}

	std::set<Connection> buildable_connections_in_network(m2::Character& player) {
		// Gather connections in player's network
		std::set<Connection> connections_in_network = get_connections_in_network(player);
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
				case FsmSignalType::EnterState:break;
				case FsmSignalType::Custom:break;
				case FsmSignalType::ExitState:break;
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
	_resource_sources = required_resources_for_network(_build_double_railroads);

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
	M2_LEVEL.add_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX, RectF{0.775f, 0.1f, 0.15f, 0.1f}, &journey_cancel_button);
	// Dim places outside the player's network
	auto buildable_conns = buildable_connections_in_network(M2_PLAYER.character());
	M2_GAME.enable_dimming_with_exceptions(M2G_PROXY.object_ids_of_connection_bg_tiles(buildable_conns));
	return std::nullopt;
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_location_mouse_click_signal(const m2::VecF& world_position) {
	LOG_DEBUG("Received mouse click", world_position);
	if (auto selected_loc = connection_on_position(world_position)) {
		LOG_INFO("Clicked on", m2g::pb::SpriteType_Name(*selected_loc));
		if (!_selected_connection_1) {
			_selected_connection_1 = *selected_loc;
		} else if (_build_double_railroads && !_selected_connection_2) {
			_selected_connection_2 = *selected_loc;
		} else {
			throw M2_ERROR("Implementation error");
		}

		// If selection done
		if (!_build_double_railroads || _selected_connection_2) {
			return _resource_sources.empty() ? NetworkJourneyStep::EXPECT_CONFIRMATION : NetworkJourneyStep::EXPECT_RESOURCE_SOURCE;
		}
	}
	LOG_DEBUG("Selected position was not on a road");
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
	M2_LEVEL.remove_custom_ui_deferred(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX);
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
			if (resource_source.first == COAL_CUBE_COUNT) {
				cc.mutable_network_action()->add_coal_sources(resource_source.second);
			} else if (resource_source.first == BEER_BARREL_COUNT) {
				cc.mutable_network_action()->set_beer_source(resource_source.second);
			} else {
				throw M2_ERROR("Unexpected resource type");
			}
		}
		M2_GAME.client_thread().queue_client_command(cc);
	} else {
		LOG_INFO("Cancelling Network action...");
	}
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}
