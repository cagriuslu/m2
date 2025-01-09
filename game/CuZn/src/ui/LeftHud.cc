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

using namespace m2;
using namespace m2::widget;
using namespace m2g;
using namespace m2g::pb;

namespace {
	const auto build_button = TextBlueprint{
		.text = "Build",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const Text& self) -> UiAction {
			if (auto build_prerequisite = CanPlayerAttemptToBuild(M2_PLAYER.character())) {
				m2g::Proxy::get_instance().main_journeys.emplace(std::in_place_type<BuildJourney>); // Avoid a temporary
			} else {
				M2_LEVEL.ShowMessage(build_prerequisite.error(), 8.0f);
			}
			return MakeContinueAction();
		}
	};

	const auto develop_button = TextBlueprint{
		.text = "Develop",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const Text& self) -> UiAction {
			if (auto develop_prerequisite = CanPlayerAttemptToDevelop(M2_PLAYER.character())) {
				m2g::Proxy::get_instance().main_journeys.emplace(std::in_place_type<DevelopJourney>);
			} else {
				M2_LEVEL.ShowMessage(develop_prerequisite.error(), 8.0f);
			}
			return MakeContinueAction();
		}
	};

	const auto loan_button = TextBlueprint{
		.text = "Loan",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const m2::widget::Text& self) -> m2::UiAction {
			if (auto loan_prerequisite = CanPlayerAttemptToLoan(M2_PLAYER.character())) {
				ExecuteLoanJourney();
			} else {
				M2_LEVEL.ShowMessage(loan_prerequisite.error(), 8.0f);
			}
			return MakeContinueAction();
		}
	};

	const auto network_button = TextBlueprint{
		.text = "Network",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const Text& self) -> UiAction {
			if (auto network_prerequisite = CanPlayerAttemptToNetwork(M2_PLAYER.character())) {
				m2g::Proxy::get_instance().main_journeys.emplace(std::in_place_type<NetworkJourney>);
			} else {
				M2_LEVEL.ShowMessage(network_prerequisite.error(), 8.0f);
			}
			return MakeContinueAction();
		}
	};

	const auto pass_button = TextBlueprint{
		.text = "Pass",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const m2::widget::Text& self) -> m2::UiAction {
			if (M2_GAME.IsOurTurn()) {
				ExecutePassJourney();
			}
			return MakeContinueAction();
		}
	};

	const auto scout_button = TextBlueprint{
		.text = "Scout",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const m2::widget::Text& self) -> m2::UiAction {
			if (auto scout_prerequisite = CanPlayerAttemptToScout(M2_PLAYER.character())) {
				ExecuteScoutJourney();
			} else {
				M2_LEVEL.ShowMessage(scout_prerequisite.error(), 8.0f);
			}
			return MakeContinueAction();
		}
	};

	const auto sell_button = TextBlueprint{
		.text = "Sell",
		.wrapped_font_size_in_units = 1.75f,
		.on_action = [](MAYBE const Text& self) -> UiAction {
			if (auto sell_prerequisite = CanPlayerAttemptToSell(M2_PLAYER.character())) {
				m2g::Proxy::get_instance().main_journeys.emplace(std::in_place_type<SellJourney>);
			} else {
				M2_LEVEL.ShowMessage(sell_prerequisite.error(), 8.0f);
			}
			return MakeContinueAction();
		}
	};
}

const UiPanelBlueprint left_hud_blueprint = {
	.name = "LeftHud",
	.w = 19,
	.h = 72,
	.border_width = 0,
	.background_color = {0, 0, 0, 255},
	.widgets = {
		UiWidgetBlueprint{
			.x = 0, .y = 0, .w = 19, .h = 4,
			.border_width = 0,
			.variant = TextBlueprint{
				.text = "Your player\nname:",
				.horizontal_alignment = m2::TextHorizontalAlignment::CENTER,
				.wrapped_font_size_in_units = 1.25f
			}
		},
		UiWidgetBlueprint{
			.x = 0, .y = 4, .w = 19, .h = 4,
			.border_width = 0,
			.variant = TextBlueprint{
				.wrapped_font_size_in_units = 2.5f,
				.on_create = [](MAYBE Text& self) {
					auto text = generate_player_name(M2_GAME.SelfIndex());
					self.set_text(text);
					self.set_color(generate_player_color(M2_GAME.SelfIndex()));
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.x = 0, .y = 8, .w = 19, .h = 10,
			.border_width = 0,
			.variant = TextBlueprint{
				.horizontal_alignment = m2::TextHorizontalAlignment::CENTER,
				.wrapped_font_size_in_units = 1.25f,
				.on_update = [](MAYBE Text& self) {
					if (M2_GAME.IsOurTurn()) {
						if (m2::is_one(M2G_PROXY.game_state_tracker().get_resource(m2g::pb::IS_LAST_ACTION_OF_PLAYER), 0.001f)) {
							self.set_text("Take your\nlast action\nof this turn");
						} else {
							self.set_text("Take your\nfirst action\nof this turn");
						}
					} else {
						if (m2::is_one(M2G_PROXY.game_state_tracker().get_resource(m2g::pb::IS_LAST_ACTION_OF_PLAYER), 0.001f)) {
							self.set_text("Current\nplayer will\ntake their\nlast action\nof this turn");
						} else {
							self.set_text("Current\nplayer will\ntake their\nfirst action\nof this turn");
						}
					}
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.name = "BuildButton",
			.initially_enabled = false,
			.x = 2, .y = 18, .w = 15, .h = 4,
			.variant = build_button
		},
		UiWidgetBlueprint{
			.name = "SellButton",
			.initially_enabled = false,
			.x = 2, .y = 23, .w = 15, .h = 4,
			.variant = sell_button
		},
		UiWidgetBlueprint{
			.name = "NetworkButton",
			.initially_enabled = false,
			.x = 2, .y = 28, .w = 15, .h = 4,
			.variant = network_button
		},
		UiWidgetBlueprint{
			.name = "LoanButton",
			.initially_enabled = false,
			.x = 2, .y = 33, .w = 15, .h = 4,
			.variant = loan_button
		},
		UiWidgetBlueprint{
			.name = "DevelopButton",
			.initially_enabled = false,
			.x = 2, .y = 38, .w = 15, .h = 4,
			.variant = develop_button
		},
		UiWidgetBlueprint{
			.name = "ScoutButton",
			.initially_enabled = false,
			.x = 2, .y = 43, .w = 15, .h = 4,
			.variant = scout_button
		},
		UiWidgetBlueprint{
			.name = "PassButton",
			.initially_enabled = false,
			.x = 2, .y = 48, .w = 15, .h = 4,
			.variant = pass_button
		}
	}
};
