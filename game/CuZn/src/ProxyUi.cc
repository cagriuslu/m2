#include <m2/Proxy.h>
#include <m2/Game.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

namespace {
	auto quit_button_action = [](MAYBE const widget::Text &self) {
		return Action::QUIT;
	};
}

static TextBlueprint client_status = {
		.initial_text = "CONNECTING",
		.on_update = [](MAYBE const Text& self) -> std::pair<Action,std::optional<std::string>> {
			if (GAME.client_thread().is_connected()) {
				return std::make_pair(Action::CONTINUE, "CONNECTED");
			} else if (GAME.client_thread().is_ready()) {
				return std::make_pair(Action::CONTINUE, "READY");
			} else if (GAME.client_thread().is_started()) {
				auto server_update = GAME.client_thread().peek_unprocessed_server_update();
				m2_succeed_or_throw_message(server_update, "Client state is Started, but ServerUpdate not found");
				auto player_count = server_update->player_object_ids_size();

				const auto expect_success = GAME.load_multi_player_as_guest(GAME.levels_dir / (std::to_string(player_count) + ".json"));
				m2_succeed_or_throw_error(expect_success);

				return std::make_pair(Action::RETURN, std::nullopt);
			} else {
				return std::make_pair(Action::CONTINUE, "CONNECTING...");
			}
		}
};
static TextBlueprint ready_button = {
		.initial_text = "...",
		.on_update = [](MAYBE const Text& self) -> std::pair<Action,std::optional<std::string>> {
			if (GAME.client_thread().is_connected()) {
				return std::make_pair(Action::CONTINUE, "SET READY");
			} else if (GAME.client_thread().is_ready()) {
				return std::make_pair(Action::CONTINUE, "CLEAR READY");
			} else {
				return std::make_pair(Action::CONTINUE, "...");
			}
		},
		.on_action = [](MAYBE const Text& self) -> Action {
			if (GAME.client_thread().is_connected()) {
				GAME.client_thread().set_ready_blocking(true);
			} else if (GAME.client_thread().is_ready()) {
				GAME.client_thread().set_ready_blocking(false);
			}
			return Action::CONTINUE;
		}
};
static const Blueprint client_lobby = {
		.w = 160, .h = 90,
		.border_width_px = 0,
		.background_color = {20, 20, 20, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 60, .y = 30, .w = 40, .h = 10,
						.border_width_px = 0,
						.variant = client_status
				},
				WidgetBlueprint{
						.x = 60, .y = 50, .w = 40, .h = 10,
						.border_width_px = 1,
						.variant = ready_button
				}
		}
};

static TextBlueprint ip_label = { .initial_text = "IP" };
static TextInputBlueprint id_addr = {
		.initial_text = "127.0.0.1"
};
static TextBlueprint connect_button = {
		.initial_text = "CONNECT",
		.on_action = [](MAYBE const widget::Text &self) {
			auto* ip_input_widget = self.parent().find_first_widget_of_type<TextInput>();
			GAME.join_game(m2::mplayer::Type::TurnBased, ip_input_widget->text_input());
			return m2::ui::State::create_execute_sync(&client_lobby);
		}
};
static const Blueprint ip_port_form = {
		.w = 160, .h = 90,
		.border_width_px = 0,
		.background_color = {20, 20, 20, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 55, .y = 30, .w = 10, .h = 10,
						.border_width_px = 0,
						.variant = ip_label
				},
				WidgetBlueprint{
						.initially_focused = true,
						.x = 65, .y = 30, .w = 40, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 5,
						.variant = id_addr
				},
				WidgetBlueprint{
						.x = 70, .y = 50, .w = 20, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 5,
						.variant = connect_button
				}
		}
};

static TextBlueprint client_count_label = { .initial_text = "Client count:" };
static TextBlueprint client_count = {
		.initial_text = "0",
		.on_create = [](MAYBE const Text& self) -> std::optional<std::string> {
			GAME.host_game(m2::mplayer::Type::TurnBased, 4);
			return std::nullopt;
		},
		.on_update = [](MAYBE const Text& self) -> std::pair<Action,std::optional<std::string>> {
			auto client_count = GAME.server_thread().client_count();
			if (client_count < 2) {
				return std::make_pair(Action::CONTINUE, std::to_string(client_count));
			} else {
				return std::make_pair(Action::CONTINUE, std::to_string(client_count) + " - START!");
			}
		},
		.on_action = [](MAYBE const Text& self) -> Action {
			if (2 <= GAME.server_thread().client_count()) {
				if (GAME.server_thread().close_lobby()) {
					const auto expect_success = GAME.load_multi_player_as_host(GAME.levels_dir / (std::to_string(GAME.server_thread().client_count()) + ".json"));
					m2_succeed_or_throw_error(expect_success);
					return Action::RETURN;
				}
			}
			return Action::CONTINUE;
		}
};
static const Blueprint server_lobby = {
		.w = 160, .h = 90,
		.border_width_px = 0,
		.background_color = {20, 20, 20, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 40, .y = 40, .w = 40, .h = 10,
						.border_width_px = 0,
						.padding_width_px = 5,
						.variant = client_count_label
				},
				WidgetBlueprint{
						.x = 80, .y = 40, .w = 40, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 5,
						.variant = client_count
				},
		}
};

static TextBlueprint join_button = {
		.initial_text = "JOIN",
		.on_action = [](MAYBE const widget::Text &self) {
			return m2::ui::State::create_execute_sync(&ip_port_form);
		}
};
static TextBlueprint host_button = {
		.initial_text = "HOST",
		.on_action = [](MAYBE const widget::Text &self) {
			return m2::ui::State::create_execute_sync(&server_lobby);
		}
};
static TextBlueprint quit_button = {
		.initial_text = "QUIT",
		.kb_shortcut = SDL_SCANCODE_Q,
		.on_action = quit_button_action
};
const Blueprint main_menu_blueprint = {
		.w = 160, .h = 90,
		.border_width_px = 0,
		.background_color = {20, 20, 20, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 70, .y = 20, .w = 20, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 5,
						.variant = join_button
				},
				WidgetBlueprint{
						.x = 70, .y = 40, .w = 20, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 5,
						.variant = host_button
				},
				WidgetBlueprint{
						.x = 70, .y = 60, .w = 20, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 5,
						.variant = quit_button
				}
		}
};

static TextBlueprint resume_button = {
		.initial_text = "RESUME",
		.on_action = [](MAYBE const widget::Text &self) {
			return Action::RETURN;
		}
};
static TextBlueprint pause_menu_quit_button = {
		.initial_text = "QUIT",
		.kb_shortcut = SDL_SCANCODE_Q,
		.on_action = quit_button_action,
};
const Blueprint pause_menu_blueprint = {
		.w = 100, .h = 100,
		.background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
		.widgets = {
				WidgetBlueprint{
						.x = 45, .y = 35, .w = 10, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 5,
						.variant = resume_button
				},
				WidgetBlueprint{
						.x = 45, .y = 55, .w = 10, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 5,
						.variant = pause_menu_quit_button
				}
		}
};

const Blueprint left_hud_blueprint = {
		.w = 19, .h = 72,
		.border_width_px = 2,
		.widgets = {
				WidgetBlueprint{
						.x = 2, .y = 2, .w = 15, .h = 8,
						.variant = TextBlueprint{
								.initial_text = "LOAN",
								.on_action = [](MAYBE const m2::ui::widget::Text& self) -> m2::ui::Action {
									if (GAME.client_thread().is_our_turn()) {
										pb::ClientCommand cc;
										cc.mutable_first_action()->mutable_loan_action();
										GAME.client_thread().queue_client_command(cc);
									}
									return Action::CONTINUE;
								}
						}
				}
		}
};

const Blueprint right_hud_blueprint = {
		.w = 19, .h = 72,
		.border_width_px = 2
};

const m2::ui::Blueprint* m2g::Proxy::main_menu() {
	return &main_menu_blueprint;
}

const m2::ui::Blueprint* m2g::Proxy::pause_menu() {
	return &pause_menu_blueprint;
}

const m2::ui::Blueprint* m2g::Proxy::left_hud() {
	return &left_hud_blueprint;
}

const m2::ui::Blueprint* m2g::Proxy::right_hud() {
	return &right_hud_blueprint;
}
