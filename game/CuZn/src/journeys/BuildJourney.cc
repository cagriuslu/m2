#include <cuzn/journeys/BuildJourney.h>
#include <m2/ui/State.h>
#include <cuzn/Detail.h>
#include <cuzn/detail/Build.h>
#include <cuzn/Ui.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::ui;
using namespace m2g;
using namespace m2g::pb;
using namespace cuzn;

cuzn::BuildJourney::BuildJourney() : m2::FsmBase<BuildJourneyStep, PositionOrCancelSignal>() {
	DEBUG_FN();
	init(BuildJourneyStep::INITIAL_STEP);
}

std::optional<cuzn::BuildJourneyStep> cuzn::BuildJourney::handle_signal(const PositionOrCancelSignal& s) {
	switch (state()) {
		case BuildJourneyStep::INITIAL_STEP:
			switch (s.type()) {
				case m2::FsmSignalType::EnterState: return handle_initial_enter_signal();
				case m2::FsmSignalType::ExitState: return std::nullopt;
				default: throw M2ERROR("Unexpected signal");
			}
		case BuildJourneyStep::EXPECT_LOCATION:
			switch (s.type()) {
				case m2::FsmSignalType::EnterState: return handle_industry_location_enter_signal();
				case m2::FsmSignalType::Custom:
					if (auto world_position = s.world_position(); world_position) {
						return handle_industry_location_mouse_click_signal(*world_position);
					} else if (s.cancel()) {
						return handle_industry_location_cancel_signal();
					}
					return std::nullopt;
				case m2::FsmSignalType::ExitState: return handle_industry_location_exit_signal();
				default: throw M2ERROR("Unexpected signal");
			}
		case BuildJourneyStep::EXPECT_RESOURCE_SOURCE:
			switch (s.type()) {
				case FsmSignalType::EnterState:
					// TODO
					break;
				case FsmSignalType::Custom:
					// TODO
					break;
				case FsmSignalType::ExitState:
					// TODO
					break;
				default: throw M2ERROR("Unexpected signal");
			}
			return std::nullopt;
		case BuildJourneyStep::EXPECT_CONFIRMATION:
			switch (s.type()) {
				case FsmSignalType::EnterState: return handle_confirmation_enter_signal();
				case FsmSignalType::Custom: return handle_confirmation_result(s.cancel());
				case FsmSignalType::ExitState: return std::nullopt;
				default: throw M2ERROR("Unexpected signal");
			}
	}
}

std::optional<BuildJourneyStep> cuzn::BuildJourney::handle_initial_enter_signal() {
	if (auto selected_card = ask_for_card_selection(); selected_card) {
		_selected_card = *selected_card;
		return BuildJourneyStep::EXPECT_LOCATION;
	} else {
		// Cancelled
		GAME.add_deferred_action(m2g::Proxy::user_journey_deleter);
		return std::nullopt;
	}
}

std::optional<BuildJourneyStep> cuzn::BuildJourney::handle_industry_location_enter_signal() {
	LOG_DEBUG("Expecting build location, disabling hud...");
	LEVEL.disable_hud();
	LEVEL.display_message("Pick location");
	LEVEL.add_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX, RectF{0.775f, 0.1f, 0.15f, 0.1f}, &journey_cancel_button);
	return std::nullopt;
}

std::optional<BuildJourneyStep> cuzn::BuildJourney::handle_industry_location_mouse_click_signal(const m2::VecF& world_position) {
	LOG_DEBUG("Received mouse click", world_position);
	if (auto selected_loc = industry_location_on_position(world_position)) {
		_selected_location = *selected_loc;
		LOG_INFO("Clicked on", m2g::pb::SpriteType_Name(_selected_location));

		// Check if there's a need to make an industry selection based on the card and the sprite
		if (auto selectable_industries = buildable_industries(_selected_card, _selected_location); selectable_industries.empty()) {
			LEVEL.display_message("Selected position cannot be built with the selected card", 10.0f);
			return std::nullopt;
		} else if (selectable_industries.size() == 2) {
			if (auto selected_industry = ask_for_industry_selection(selectable_industries[0], selectable_industries[1]); selected_industry) {
				_selected_industry = *selected_industry;
			} else {
				// Cancelled
				GAME.add_deferred_action(m2g::Proxy::user_journey_deleter);
				return std::nullopt;
			}
		} else if (selectable_industries.size() == 1) {
			_selected_industry = selectable_industries[0];
		} else {
			throw M2ERROR("Implementation error, more than 2 selectable industries");
		}

		// Check if the player can build the selected industry
		if (auto tile_type = can_player_build_industry(LEVEL.player()->character(), _selected_card, _selected_location, _selected_industry); not tile_type) {
			LOG_INFO("Cancelling Build action...");
			LEVEL.display_message(tile_type.error());
			GAME.add_deferred_action(m2g::Proxy::user_journey_deleter);
			return std::nullopt;
		} else {
			// Create empty entries in resource_sources for every required resource
			_resource_sources.insert(_resource_sources.end(),
				iround(GAME.get_named_item(*tile_type).get_attribute(m2g::pb::COAL_COST)),
				std::make_pair(m2g::pb::COAL_CUBE_COUNT, NO_SPRITE));
			_resource_sources.insert(_resource_sources.end(),
				iround(GAME.get_named_item(*tile_type).get_attribute(m2g::pb::IRON_COST)),
				std::make_pair(m2g::pb::IRON_CUBE_COUNT, NO_SPRITE));
			return _resource_sources.empty() ? BuildJourneyStep::EXPECT_CONFIRMATION : BuildJourneyStep::EXPECT_RESOURCE_SOURCE;
		}
	}
	LOG_DEBUG("Selected position was not on an industry");
	return std::nullopt;
}

std::optional<BuildJourneyStep> cuzn::BuildJourney::handle_industry_location_cancel_signal() {
	LOG_INFO("Cancelling Build action...");
	deinit();
	GAME.add_deferred_action(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}

std::optional<BuildJourneyStep> cuzn::BuildJourney::handle_industry_location_exit_signal() {
	LOG_DEBUG("Re-enabling hud...");
	LEVEL.enable_hud();
	LEVEL.remove_message();
	LEVEL.remove_custom_ui_deferred(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX);
	return std::nullopt;
}

std::optional<BuildJourneyStep> cuzn::BuildJourney::handle_confirmation_enter_signal() {
	LOG_INFO("Asking for confirmation...");
	LEVEL.add_custom_ui_dialog({0.2f, 0.2f, 0.6f, 0.6f}, std::make_unique<m2::ui::Blueprint>(
		generate_build_confirmation(_selected_card, city_of_location(_selected_location), _selected_industry)
	));
	return std::nullopt;
}

std::optional<BuildJourneyStep> cuzn::BuildJourney::handle_confirmation_result(bool cancelled) {
	if (cancelled) {
		LOG_INFO("Cancelling Build action...");
	} else {
		LOG_INFO("Build action confirmed");
		LEVEL.display_message("Building location...");

		m2g::pb::ClientCommand cc;
		cc.mutable_build_action()->set_card(_selected_card);
		cc.mutable_build_action()->set_location(_selected_location);
		cc.mutable_build_action()->set_industry(_selected_industry);
		for (const auto& resource_source : _resource_sources) {
			if (resource_source.first == COAL_CUBE_COUNT) {
				cc.mutable_build_action()->add_coal_sources(resource_source.second);
			} else if (resource_source.first == IRON_CUBE_COUNT) {
				cc.mutable_build_action()->add_iron_sources(resource_source.second);
			} else {
				throw M2ERROR("Unexpected resource type");
			}
		}
		GAME.client_thread().queue_client_command(cc);
	}
	GAME.add_deferred_action(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}
