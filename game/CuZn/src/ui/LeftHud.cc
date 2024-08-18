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
		.font_size = 4.5f,
		.on_action = [](MAYBE const Text& self) -> Action {
			if (auto build_prerequisite = can_player_attempt_to_build(M2_PLAYER.character())) {
				m2g::Proxy::get_instance().user_journey.emplace(BuildJourney{});
			} else {
				M2_LEVEL.display_message(build_prerequisite.error());
			}
			return make_continue_action();
		}
	};

	const auto develop_button = TextBlueprint{
		.text = "Develop",
		.font_size = 4.5f,
		.on_action = [](MAYBE const Text& self) -> Action {
			if (auto develop_prerequisite = can_player_attempt_to_develop(M2_PLAYER.character())) {
				m2g::Proxy::get_instance().user_journey.emplace(DevelopJourney{});
			} else {
				M2_LEVEL.display_message(develop_prerequisite.error());
			}
			return make_continue_action();
		}
	};

	const auto loan_button = TextBlueprint{
		.text = "Loan",
		.font_size = 4.5f,
		.on_action = [](MAYBE const m2::ui::widget::Text& self) -> m2::ui::Action {
			if (auto loan_prerequisite = can_player_attempt_to_loan(M2_PLAYER.character())) {
				execute_loan_journey();
			} else {
				M2_LEVEL.display_message(loan_prerequisite.error());
			}
			return make_continue_action();
		}
	};

	const auto network_button = TextBlueprint{
		.text = "Network",
		.font_size = 4.5f,
		.on_action = [](MAYBE const Text& self) -> Action {
			if (auto network_prerequisite = can_player_attempt_to_network(M2_PLAYER.character())) {
				m2g::Proxy::get_instance().user_journey.emplace(NetworkJourney{});
			} else {
				M2_LEVEL.display_message(network_prerequisite.error());
			}
			return make_continue_action();
		}
	};

	const auto pass_button = TextBlueprint{
		.text = "Pass",
		.font_size = 4.5f,
		.on_action = [](MAYBE const m2::ui::widget::Text& self) -> m2::ui::Action {
			if (M2_GAME.client_thread().is_turn()) {
				execute_pass_journey();
			}
			return make_continue_action();
		}
	};

	const auto scout_button = TextBlueprint{
		.text = "Scout",
		.font_size = 4.5f,
		.on_action = [](MAYBE const m2::ui::widget::Text& self) -> m2::ui::Action {
			if (auto scout_prerequisite = can_player_attempt_to_scout(M2_PLAYER.character())) {
				execute_scout_journey();
			} else {
				M2_LEVEL.display_message(scout_prerequisite.error());
			}
			return make_continue_action();
		}
	};

	const auto sell_button = TextBlueprint{
		.text = "Sell",
		.font_size = 4.5f,
		.on_action = [](MAYBE const Text& self) -> Action {
			if (auto sell_prerequisite = can_player_attempt_to_sell(M2_PLAYER.character())) {
				m2g::Proxy::get_instance().main_journeys.emplace(SellJourney{});
			} else {
				M2_LEVEL.display_message(sell_prerequisite.error());
			}
			return make_continue_action();
		}
	};
}

const Blueprint left_hud_blueprint = {
	.w = 19,
	.h = 72,
	.border_width_px = 0,
	.background_color = {0, 0, 0, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 2, .y = 2, .w = 15, .h = 6,
			.border_width_px = 0,
			.variant = TextBlueprint{
				.font_size = 4.5f,
				.alignment = m2::ui::TextAlignment::LEFT,
				.on_update = [](MAYBE Text& self) {
					auto text = std::string{"Color:"} + generate_player_name(M2_GAME.client_thread().receiver_index());
					self.set_text(text);
					self.set_color(generate_player_color(M2_GAME.client_thread().receiver_index()));
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 2, .y = 9, .w = 15, .h = 6,
			.border_width_px = 0,
			.variant = TextBlueprint{
				.font_size = 4.5f,
				.alignment = m2::ui::TextAlignment::LEFT,
				.on_update = [](MAYBE Text& self) {
					if (M2_GAME.client_thread().is_turn()) {
						self.set_text("Your turn");
					} else {
						self.set_text("Wait turn");
					}
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.name = "BuildButton",
			.initially_enabled = false,
			.x = 2, .y = 16, .w = 15, .h = 6,
			.variant = build_button
		},

		WidgetBlueprint{
			.name = "SellButton",
			.initially_enabled = false,
			.x = 2, .y = 23, .w = 15, .h = 6,
			.variant = sell_button
		},
		WidgetBlueprint{
			.name = "NetworkButton",
			.initially_enabled = false,
			.x = 2, .y = 30, .w = 15, .h = 6,
			.variant = network_button
		},
		WidgetBlueprint{
			.name = "LoanButton",
			.initially_enabled = false,
			.x = 2, .y = 37, .w = 15, .h = 6,
			.variant = loan_button
		},
		WidgetBlueprint{
			.name = "DevelopButton",
			.initially_enabled = false,
			.x = 2, .y = 44, .w = 15, .h = 6,
			.variant = develop_button
		},
		WidgetBlueprint{
			.name = "ScoutButton",
			.initially_enabled = false,
			.x = 2, .y = 51, .w = 15, .h = 6,
			.variant = scout_button
		},
		WidgetBlueprint{
			.name = "PassButton",
			.initially_enabled = false,
			.x = 2, .y = 58, .w = 15, .h = 6,
			.variant = pass_button
		},
	}
};
