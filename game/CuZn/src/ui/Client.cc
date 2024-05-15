#include <cuzn/ui/Client.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/TextListSelection.h>
#include <m2/Game.h>
#include <m2/Log.h>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

static TextBlueprint client_status = {
	.initial_text = "CONNECTING",
	.on_update = [](MAYBE const Text& self) -> std::pair<Action, std::optional<std::string>> {
		if (M2_GAME.client_thread().is_connected()) {
			return std::make_pair(make_continue_action(), "CONNECTED");
		} else if (M2_GAME.client_thread().is_ready()) {
			return std::make_pair(make_continue_action(), "READY");
		} else if (M2_GAME.client_thread().is_started()) {
			auto server_update = M2_GAME.client_thread().peek_unprocessed_server_update();
			m2_succeed_or_throw_message(server_update, "Client state is Started, but ServerUpdate not found");
			auto player_count = server_update->player_object_ids_size();

			const auto expect_success =
				M2_GAME.load_multi_player_as_guest(M2_GAME.levels_dir / "Map.json", std::to_string(player_count));
			m2_succeed_or_throw_error(expect_success);

			return std::make_pair(make_return_action(), std::nullopt);
		} else {
			return std::make_pair(make_continue_action(), "CONNECTING...");
		}
	}
};

static TextBlueprint ready_button = {
	.initial_text = "...",
	.on_update = [](MAYBE const Text& self) -> std::pair<Action, std::optional<std::string>> {
		if (M2_GAME.client_thread().is_connected()) {
			return std::make_pair(make_continue_action(), "SET READY");
		} else if (M2_GAME.client_thread().is_ready()) {
			return std::make_pair(make_continue_action(), "CLEAR READY");
		} else {
			return std::make_pair(make_continue_action(), "...");
		}
	},
	.on_action = [](MAYBE const Text& self) -> Action {
		if (M2_GAME.client_thread().is_connected()) {
			M2_GAME.client_thread().set_ready_blocking(true);
		} else if (M2_GAME.client_thread().is_ready()) {
			M2_GAME.client_thread().set_ready_blocking(false);
		}
		return make_continue_action();
	}
};

static const Blueprint client_lobby = {
	.w = 160,
	.h = 90,
	.border_width_px = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		WidgetBlueprint{.x = 60, .y = 30, .w = 40, .h = 10, .border_width_px = 0, .variant = client_status},
		WidgetBlueprint{.x = 60, .y = 50, .w = 40, .h = 10, .border_width_px = 1, .variant = ready_button}
	}
};

const Blueprint cuzn::ip_port_form = {
	.w = 160,
	.h = 90,
	.border_width_px = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		WidgetBlueprint{.x = 55, .y = 30, .w = 10, .h = 10,
			.border_width_px = 0,
			.variant = TextBlueprint{.initial_text = "IP"}
		},
		WidgetBlueprint{
			.initially_focused = true,
			.x = 65,
			.y = 30,
			.w = 40,
			.h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = TextInputBlueprint{.initial_text = "127.0.0.1"}
		},
		WidgetBlueprint{
			.x = 70,
			.y = 50,
			.w = 20,
			.h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = TextBlueprint{
				.initial_text = "CONNECT", .on_action = [](MAYBE const widget::Text& self) {
					auto* ip_input_widget = self.parent().find_first_widget_of_type<TextInput>();
					M2_GAME.join_game(m2::mplayer::Type::TurnBased, ip_input_widget->text_input());
					return m2::ui::State::create_execute_sync(&client_lobby);
				}
			}
		}
	}
};
