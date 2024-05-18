#include <cuzn/ui/LeftHud.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/TextListSelection.h>
#include <m2/Game.h>
#include <m2/Log.h>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

namespace {
	const auto build_button = TextBlueprint{
		.initial_text = "Build",
		.font_size = 4.5f,
		.on_action = [](MAYBE const Text& self) -> Action {
			if (M2_GAME.client_thread().is_our_turn()) {
				LOG_INFO("Beginning BuildJourney");
				m2g::Proxy::get_instance().user_journey.emplace(cuzn::BuildJourney{});
			} else {
				M2_LEVEL.display_message("It's not your turn");
			}
			return make_continue_action();
		}
	};

	const auto network_button = TextBlueprint{
		.initial_text = "Network",
		.font_size = 4.5f,
		.on_action = [](MAYBE const Text& self) -> Action {
			if (M2_GAME.client_thread().is_our_turn()) {
				LOG_INFO("Beginning BuildJourney");
				m2g::Proxy::get_instance().user_journey.emplace(cuzn::NetworkJourney{});
			} else {
				M2_LEVEL.display_message("It's not your turn");
			}
			return make_continue_action();
		}
	};

	const auto develop_button = TextBlueprint{.initial_text = "Develop", .font_size = 4.5f};

	const auto sell_button = TextBlueprint{.initial_text = "Sell", .font_size = 4.5f};

	const auto loan_button = TextBlueprint{
		.initial_text = "Loan",
		.font_size = 4.5f,
		.on_action = [](MAYBE const m2::ui::widget::Text& self) -> m2::ui::Action {
			if (M2_GAME.client_thread().is_our_turn()) {
				pb::ClientCommand cc;
				cc.mutable_loan_action();
				M2_GAME.client_thread().queue_client_command(cc);
			}
			return make_continue_action();
		}
	};

	const auto scout_button = TextBlueprint{.initial_text = "Scout", .font_size = 4.5f};
}

const Blueprint cuzn::left_hud_blueprint = {
	.w = 19,
	.h = 72,
	.border_width_px = 0,
	.background_color = {0, 0, 0, 255},
	.widgets = {
		WidgetBlueprint{.x = 2, .y = 2, .w = 15, .h = 6, .variant = build_button},
		WidgetBlueprint{.x = 2, .y = 9, .w = 15, .h = 6, .variant = network_button},
		WidgetBlueprint{.x = 2, .y = 16, .w = 15, .h = 6, .variant = develop_button},
		WidgetBlueprint{.x = 2, .y = 23, .w = 15, .h = 6, .variant = sell_button},
		WidgetBlueprint{.x = 2, .y = 30, .w = 15, .h = 6, .variant = loan_button},
		WidgetBlueprint{.x = 2, .y = 37, .w = 15, .h = 6, .variant = scout_button}
	}
};