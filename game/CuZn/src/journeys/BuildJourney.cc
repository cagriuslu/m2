#include <cuzn/journeys/BuildJourney.h>
#include <m2/ui/State.h>
#include <cuzn/Detail.h>
#include <cuzn/Build.h>
#include <cuzn/Ui.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::ui;
using namespace m2g;
using namespace m2g::pb;
using namespace cuzn;

namespace {
	const Blueprint cancel_button{
		.border_width_px = 0,
		.widgets = {
			WidgetBlueprint{
				.background_color = {0, 0, 0, 255},
				.variant = widget::TextBlueprint{
					.initial_text = "Cancel",
					.on_action = [](MAYBE const widget::Text& self) -> Action {
						// Create and send a cancel signal to the current user journey
						auto& user_journey = m2g::Proxy::get_instance().user_journey;
						// Check if BuildJourney is active
						if (std::holds_alternative<BuildJourney>(*user_journey)) {
							// Deliver cancel signal to BuildJourney
							std::get<BuildJourney>(*user_journey).signal(BuildJourneySignal::create_cancel_signal());
						}
						return make_return_action();
					}
				}
			}
		}
	};
	constexpr int CANCEL_BUTTON_CUSTOM_UI_INDEX = 0;
}

BuildJourneySignal BuildJourneySignal::create_mouse_click_signal(m2::VecF world_position) {
	auto signal = BuildJourneySignal{m2::FsmSignalType::Custom};
	signal._world_position = world_position;
	return signal;
}

BuildJourneySignal BuildJourneySignal::create_cancel_signal(bool cancelled) {
	auto signal = BuildJourneySignal{m2::FsmSignalType::Custom};
	signal._cancel = cancelled;
	return signal;
}

cuzn::BuildJourney::BuildJourney() : m2::FsmBase<BuildJourneyStep, BuildJourneySignal>() {
	DEBUG_FN();
	init(BuildJourneyStep::INITIAL_STEP);
}

std::optional<cuzn::BuildJourneyStep> cuzn::BuildJourney::handle_signal(const BuildJourneySignal& s) {
	switch (state()) {
		case BuildJourneyStep::INITIAL_STEP:
			switch (s.type()) {
				case m2::FsmSignalType::EnterState: return handle_initial_enter_signal();
				case m2::FsmSignalType::ExitState: return std::nullopt;
				default: throw M2ERROR("Unexpected signal");
			}
		case BuildJourneyStep::EXPECT_INDUSTRY_LOCATION:
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
	return std::nullopt;
}

std::optional<BuildJourneyStep> cuzn::BuildJourney::handle_initial_enter_signal() {
	LOG_INFO("Asking player to pick a card...");
	std::optional<BuildJourneyStep> next_state;
	m2::ui::State::create_execute_sync(std::make_unique<m2::ui::Blueprint>(generate_cards_window(true)), GAME.dimensions().game_and_hud.ratio({0.15f, 0.15f, 0.7f, 0.7f}))
		.if_void_return([&]() {
			LOG_INFO("Cancelling Build action...");
			GAME.add_deferred_action(m2g::Proxy::user_journey_deleter);
			next_state = std::nullopt;
		})
		.if_return<m2g::pb::ItemType>([&](auto picked_card) {
			LOG_INFO("Selected card", m2g::pb::ItemType_Name(picked_card));
			_selected_card = picked_card;
			next_state = BuildJourneyStep::EXPECT_INDUSTRY_LOCATION;
		});
	return next_state;
}

std::optional<BuildJourneyStep> cuzn::BuildJourney::handle_industry_location_enter_signal() {
	LOG_DEBUG("Expecting build location, disabling hud...");
	LEVEL.left_hud_ui_state->enabled = false;
	LEVEL.right_hud_ui_state->enabled = false;
	LEVEL.display_message("Pick location");
	LEVEL.add_custom_ui(CANCEL_BUTTON_CUSTOM_UI_INDEX, RectF{0.775f, 0.1f, 0.15f, 0.1f}, &cancel_button);
	return std::nullopt;
}

std::optional<BuildJourneyStep> cuzn::BuildJourney::handle_industry_location_mouse_click_signal(const m2::VecF& world_position) {
	LOG_DEBUG("Received mouse click", world_position);
	if (auto selected_loc = std::find_if(PROXY.industry_positions.begin(), PROXY.industry_positions.end(),
			[&](const auto& pos_and_type) { return pos_and_type.second.point_in_rect(world_position); });
		selected_loc != PROXY.industry_positions.end()) {
		_selected_location = selected_loc->first;
		LOG_INFO("Clicked on", m2g::pb::SpriteType_Name(_selected_location));

		// Check if there's a need to make an industry selection based on the card and the sprite
		if (auto selectable_industries = cuzn::selectable_industries(_selected_card, _selected_location); selectable_industries.empty()) {
			LEVEL.display_message("Selected position cannot be built with the selected card", 10.0f);
			return std::nullopt;
		} else if (selectable_industries.size() == 2) {
			LOG_INFO("Asking player to pick an industry...");
			bool cancelled{};
			m2::ui::State::create_execute_sync(
				std::make_unique<m2::ui::Blueprint>(generate_industry_selection_window(selectable_industries[0], selectable_industries[1])),
				GAME.dimensions().game_and_hud.ratio({0.15f, 0.15f, 0.7f, 0.7f}))
				.if_void_return([&]() {
					LOG_INFO("Cancelling Build action...");
					GAME.add_deferred_action(m2g::Proxy::user_journey_deleter);
					cancelled = true;
				})
				.if_return<m2g::pb::ItemType>([&](auto selected_industry) {
					LOG_INFO("Selected industry", m2g::pb::ItemType_Name(selected_industry));
					_selected_industry = selected_industry;
				});
			if (cancelled) {
				return std::nullopt;
			}
		} else if (selectable_industries.size() == 1) {
			_selected_industry = selectable_industries[0];
		} else {
			throw M2ERROR("Implementation error, more than 2 selectable industries");
		}

		// Check if the player can build the selected industry
		if (auto tile_type = can_player_build_industry(LEVEL.player()->character(), _selected_location, _selected_industry); not tile_type) {
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
	LEVEL.left_hud_ui_state->enabled = true;
	LEVEL.right_hud_ui_state->enabled = true;
	LEVEL.remove_message();
	LEVEL.remove_custom_ui_deferred(CANCEL_BUTTON_CUSTOM_UI_INDEX);
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
