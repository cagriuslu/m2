#include <cuzn/ui/Server.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/TextListSelection.h>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

static TextBlueprint client_count = {
	.initial_text = "0",
	.on_update = [](MAYBE const Text& self) -> std::pair<Action, std::optional<std::string>> {
		auto client_count = M2_GAME.server_thread().client_count();
		if (client_count < 2) {
			return std::make_pair(make_continue_action(), std::to_string(client_count));
		} else {
			return std::make_pair(make_continue_action(), std::to_string(client_count) + " - START!");
		}
	},
	.on_action = [](MAYBE const Text& self) -> Action {
		if (2 <= M2_GAME.server_thread().client_count()) {
			LOG_INFO("Enough clients have connected");
			if (M2_GAME.server_thread().close_lobby()) {
				auto client_count = M2_GAME.server_thread().client_count();
				const auto expect_success =
					M2_GAME.load_multi_player_as_host(M2_GAME.levels_dir / "Map.json", std::to_string(client_count));
				m2_succeed_or_throw_error(expect_success);
				return make_return_action();  // TODO Return value
			}
		}
		return make_continue_action();
	}
};

const Blueprint cuzn::server_lobby = {
	.w = 160,
	.h = 90,
	.border_width_px = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 40,
			.y = 40,
			.w = 40,
			.h = 10,
			.border_width_px = 0,
			.padding_width_px = 5,
			.variant = TextBlueprint{.initial_text = "Client count:"}
		},
		WidgetBlueprint{
			.x = 80, .y = 40, .w = 40, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = client_count
		},
	}
};
