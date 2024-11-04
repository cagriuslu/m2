#include <cuzn/ui/LeftHud.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/journeys/DevelopJourney.h>
#include <cuzn/journeys/LoanJourney.h>
#include <cuzn/journeys/ScoutJourney.h>
#include <cuzn/journeys/PassJourney.h>
#include <cuzn/journeys/SellJourney.h>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

namespace {
	const auto build_button = TextBlueprint{
		.text = "Build",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const Text& self) -> Action {
			if (auto build_prerequisite = can_player_attempt_to_build(M2_PLAYER.character())) {
				m2g::Proxy::get_instance().main_journeys.emplace(std::in_place_type<BuildJourney>); // Avoid a temporary
			} else {
				M2G_PROXY.show_notification(build_prerequisite.error());
			}
			return make_continue_action();
		}
	};

	const auto develop_button = TextBlueprint{
		.text = "Develop",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const Text& self) -> Action {
			if (auto develop_prerequisite = can_player_attempt_to_develop(M2_PLAYER.character())) {
				m2g::Proxy::get_instance().user_journey.emplace(std::in_place_type<DevelopJourney>);
			} else {
				M2G_PROXY.show_notification(develop_prerequisite.error());
			}
			return make_continue_action();
		}
	};

	const auto loan_button = TextBlueprint{
		.text = "Loan",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const m2::ui::widget::Text& self) -> m2::ui::Action {
			if (auto loan_prerequisite = can_player_attempt_to_loan(M2_PLAYER.character())) {
				execute_loan_journey();
			} else {
				M2G_PROXY.show_notification(loan_prerequisite.error());
			}
			return make_continue_action();
		}
	};

	const auto network_button = TextBlueprint{
		.text = "Network",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const Text& self) -> Action {
			if (auto network_prerequisite = can_player_attempt_to_network(M2_PLAYER.character())) {
				m2g::Proxy::get_instance().user_journey.emplace(std::in_place_type<NetworkJourney>);
			} else {
				M2G_PROXY.show_notification(network_prerequisite.error());
			}
			return make_continue_action();
		}
	};

	const auto pass_button = TextBlueprint{
		.text = "Pass",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const m2::ui::widget::Text& self) -> m2::ui::Action {
			if (M2_GAME.is_our_turn()) {
				execute_pass_journey();
			}
			return make_continue_action();
		}
	};

	const auto scout_button = TextBlueprint{
		.text = "Scout",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const m2::ui::widget::Text& self) -> m2::ui::Action {
			if (auto scout_prerequisite = can_player_attempt_to_scout(M2_PLAYER.character())) {
				execute_scout_journey();
			} else {
				M2G_PROXY.show_notification(scout_prerequisite.error());
			}
			return make_continue_action();
		}
	};

	const auto sell_button = TextBlueprint{
		.text = "Sell",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const Text& self) -> Action {
			if (auto sell_prerequisite = can_player_attempt_to_sell(M2_PLAYER.character())) {
				m2g::Proxy::get_instance().main_journeys.emplace(std::in_place_type<SellJourney>);
			} else {
				M2G_PROXY.show_notification(sell_prerequisite.error());
			}
			return make_continue_action();
		}
	};
}

const PanelBlueprint left_hud_blueprint = {
	.w = 19,
	.h = 72,
	.border_width = 0,
	.background_color = {0, 0, 0, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 0, .y = 0, .w = 19, .h = 4,
			.border_width = 0,
			.variant = TextBlueprint{
				.text = "Your player\nname:",
				.horizontal_alignment = m2::ui::TextHorizontalAlignment::CENTER,
				.wrapped_font_size_in_units = 1.35f
			}
		},
		WidgetBlueprint{
			.x = 0, .y = 4, .w = 19, .h = 4,
			.border_width = 0,
			.variant = TextBlueprint{
				.wrapped_font_size_in_units = 2.5f,
				.on_create = [](MAYBE Text& self) {
					auto text = generate_player_name(M2_GAME.self_index());
					self.set_text(text);
					self.set_color(generate_player_color(M2_GAME.self_index()));
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 0, .y = 8, .w = 19, .h = 10,
			.border_width = 0,
			.variant = TextBlueprint{
				.horizontal_alignment = m2::ui::TextHorizontalAlignment::CENTER,
				.wrapped_font_size_in_units = 1.35f,
				.on_update = [](MAYBE Text& self) {
					if (M2_GAME.is_our_turn()) {
						if (m2::is_one(M2G_PROXY.game_state_tracker().get_resource(pb::IS_LAST_ACTION_OF_PLAYER), 0.001f)) {
							self.set_text("Take your\nlast action\nof this turn");
						} else {
							self.set_text("Take your\nfirst action\nof this turn");
						}
					} else {
						if (m2::is_one(M2G_PROXY.game_state_tracker().get_resource(pb::IS_LAST_ACTION_OF_PLAYER), 0.001f)) {
							self.set_text("Current\nplayer will\ntake their\nlast action\nof this turn");
						} else {
							self.set_text("Current\nplayer will\ntake their\nfirst action\nof this turn");
						}
					}
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.name = "BuildButton",
			.initially_enabled = false,
			.x = 2, .y = 18, .w = 15, .h = 4,
			.variant = build_button
		},
		WidgetBlueprint{
			.name = "SellButton",
			.initially_enabled = false,
			.x = 2, .y = 23, .w = 15, .h = 4,
			.variant = sell_button
		},
		WidgetBlueprint{
			.name = "NetworkButton",
			.initially_enabled = false,
			.x = 2, .y = 28, .w = 15, .h = 4,
			.variant = network_button
		},
		WidgetBlueprint{
			.name = "LoanButton",
			.initially_enabled = false,
			.x = 2, .y = 33, .w = 15, .h = 4,
			.variant = loan_button
		},
		WidgetBlueprint{
			.name = "DevelopButton",
			.initially_enabled = false,
			.x = 2, .y = 38, .w = 15, .h = 4,
			.variant = develop_button
		},
		WidgetBlueprint{
			.name = "ScoutButton",
			.initially_enabled = false,
			.x = 2, .y = 43, .w = 15, .h = 4,
			.variant = scout_button
		},
		WidgetBlueprint{
			.name = "PassButton",
			.initially_enabled = false,
			.x = 2, .y = 48, .w = 15, .h = 4,
			.variant = pass_button
		}
	}
};
