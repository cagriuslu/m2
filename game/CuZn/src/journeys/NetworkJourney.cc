#include <cuzn/journeys/NetworkJourney.h>
#include <cuzn/Detail.h>
#include <cuzn/detail/Network.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2g/Proxy.h>

using namespace m2;
using namespace m2::ui;
using namespace m2g;
using namespace m2g::pb;
using namespace cuzn;

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
				default: throw M2ERROR("Unexpected signal");
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
					throw M2ERROR("Unexpected signal");
				}
				case FsmSignalType::ExitState: return handle_location_exit_signal();
				default: throw M2ERROR("Unexpected signal");
			}
		case NetworkJourneyStep::EXPECT_RESOURCE_SOURCE:
			switch (s.type()) {
				case FsmSignalType::EnterState:break;
				case FsmSignalType::Custom:break;
				case FsmSignalType::ExitState:break;
				default: throw M2ERROR("Unexpected signal");
			}
		case NetworkJourneyStep::EXPECT_CONFIRMATION:
			switch (s.type()) {
				case FsmSignalType::EnterState: return handle_confirmation_enter_signal();
				case FsmSignalType::ExitState: return std::nullopt;
				default: throw M2ERROR("Unexpected signal");
			}
	}
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_initial_enter_signal() {
	if (player_card_count(M2_PLAYER.character()) == 0) {
		throw M2ERROR("Player has no cards but NetworkJourney is triggered. The game should have ended instead");
	}
	if (player_road_count(M2_PLAYER.character()) == 0) {
		M2_LEVEL.display_message("You are out of road tiles.");
		LOG_INFO("Insufficient roads, cancelling NetworkJourney...");
		M2_DEFER(m2g::Proxy::user_journey_deleter);
		return std::nullopt;
	}

	// Ask if double railroads should be built
	if (is_railroad_era() && 1 < player_card_count(M2_PLAYER.character()) && 1 < player_road_count(M2_PLAYER.character())) {
		_build_double_railroads = ask_for_confirmation("Build double railroads?", "", "Yes", "No");
	}

	// Check player money, calculate required resources
	if (auto costs = road_costs(_build_double_railroads);
		player_money(M2_PLAYER.character()) < (costs
			| std::views::filter(m2::is_first_equals<m2g::pb::ResourceType, float>(MONEY))
			| std::views::transform(m2::to_second_of<m2g::pb::ResourceType, float>)).front()) {
		M2_LEVEL.display_message("Insufficient money.");
		LOG_INFO("Insufficient money, cancelling NetworkJourney...");
		M2_DEFER(m2g::Proxy::user_journey_deleter);
		return std::nullopt;
	} else {
		for (const auto& cost : costs | std::views::filter(m2::is_first_not_equals<m2g::pb::ResourceType, float>(MONEY))) {
			_resource_sources.insert(_resource_sources.end(), iround(cost.second), std::make_pair(cost.first, NO_SPRITE));
		}
	}

	// Card selection
	if (auto selected_card_1 = ask_for_card_selection(); selected_card_1) {
		_selected_card_1 = *selected_card_1;
		if (_build_double_railroads) {
			if (auto selected_card_2 = ask_for_card_selection(_selected_card_1); selected_card_2) {
				_selected_card_2 = *selected_card_2;
			} else {
				M2_DEFER(m2g::Proxy::user_journey_deleter);
				return std::nullopt;
			}
		}
	} else {
		M2_DEFER(m2g::Proxy::user_journey_deleter);
		return std::nullopt;
	}

	return NetworkJourneyStep::EXPECT_LOCATION;
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_location_enter_signal() {
	LOG_DEBUG("Expecting industry location...");
	M2_LEVEL.disable_hud();
	M2_LEVEL.display_message("Pick location", -1.0f);
	M2_LEVEL.add_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX, RectF{0.775f, 0.1f, 0.15f, 0.1f}, &journey_cancel_button);
	return std::nullopt;
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_location_mouse_click_signal(const m2::VecF& world_position) {
	LOG_DEBUG("Received mouse click", world_position);
	if (auto selected_loc = connection_on_position(world_position)) {
		LOG_INFO("Clicked on", m2g::pb::SpriteType_Name(*selected_loc));
		if (!_selected_location_1) {
			_selected_location_1 = *selected_loc;
		} else if (_build_double_railroads && !_selected_location_2) {
			_selected_location_2 = *selected_loc;
		} else {
			throw M2ERROR("Implementation error");
		}

		// If selection done
		if (!_build_double_railroads || _selected_location_2) {
			return _resource_sources.empty() ? NetworkJourneyStep::EXPECT_CONFIRMATION : NetworkJourneyStep::EXPECT_RESOURCE_SOURCE;
		}
	}
	LOG_DEBUG("Selected position was not on a road");
	return std::nullopt;
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_location_cancel_signal() {
	LOG_INFO("Cancelling Network action...");
	deinit();
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_location_exit_signal() {
	M2_LEVEL.enable_hud();
	M2_LEVEL.remove_message();
	M2_LEVEL.remove_custom_ui_deferred(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX);
	return std::nullopt;
}

std::optional<NetworkJourneyStep> NetworkJourney::handle_confirmation_enter_signal() {
	LOG_INFO("Asking for confirmation...");
	// TODO
	if (ask_for_confirmation("Are you sure?", "", "OK", "Cancel")) {
		LOG_INFO("Network action confirmed");

		m2g::pb::ClientCommand cc;
		cc.mutable_network_action();
		M2_GAME.client_thread().queue_client_command(cc);
	} else {
		LOG_INFO("Cancelling Network action...");
	}
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}
