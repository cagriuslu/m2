#include <cuzn/ui/Server.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/network/Detail.h>
#include <numeric>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

static TextBlueprint client_count = {
	.text = "0",
	.wrapped_font_size_in_units = 5.0f,
	.on_update = [](MAYBE Text& self) {
		auto client_count = M2_GAME.server_thread().client_count();
		auto ready_client_count = M2_GAME.server_thread().ready_client_count();
		auto text = std::to_string(ready_client_count) + "/" + std::to_string(client_count);
		// Check if ready to start
		if (client_count != 1 && client_count == ready_client_count) {
			text += " - START!";
		}
		self.set_text(text);

		return make_continue_action();
	},
	.on_action = [](MAYBE const Text& self) -> Action {
		if (2 <= M2_GAME.server_thread().client_count()) {
			LOG_INFO("Enough clients have connected");
			if (M2_GAME.server_thread().close_lobby()) {
				auto client_count = M2_GAME.server_thread().client_count();
				const auto expect_success = M2_GAME.load_multi_player_as_host(M2_GAME.levels_dir / "Map.json", std::to_string(client_count));
				m2_succeed_or_throw_error(expect_success);
				return make_clear_stack_action();
			}
		}
		return make_continue_action();
	}
};

const PanelBlueprint server_lobby = {
	.w = 160,
	.h = 90,
	.border_width = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 35, .y = 5, .w = 40, .h = 20,
			.border_width = 0,
			.variant = TextBlueprint{
				.text = "LISTENING ON:",
				.horizontal_alignment = m2::ui::TextHorizontalAlignment::RIGHT,
				.vertical_alignment = m2::ui::TextVerticalAlignment::CENTER,
				.wrapped_font_size_in_units = 5.0f
			}
		},
		WidgetBlueprint{
			.x = 85, .y = 5, .w = 60, .h = 20,
			.border_width = 0,
			.variant = TextBlueprint{
				.horizontal_alignment = m2::ui::TextHorizontalAlignment::LEFT,
				.vertical_alignment = m2::ui::TextVerticalAlignment::CENTER,
				.wrapped_font_size_in_units = 5.0f,
				.on_create = [](Text& self) {
					if (auto addresses = m2::network::get_ip_addresses()) {
						auto addresses_str = std::accumulate(addresses->begin(), addresses->end(), std::string{}, [](std::string&& ss, const std::string& s) {
							return ss.empty() ? s : (ss + " " + s);
						});
						self.set_text(addresses_str);
					}
				}
			}
		},
		WidgetBlueprint{
			.x = 35, .y = 35, .w = 40, .h = 10,
			.border_width = 0,
			.variant = TextBlueprint{
				.text = "CLIENT COUNT:",
				.horizontal_alignment = m2::ui::TextHorizontalAlignment::RIGHT,
				.wrapped_font_size_in_units = 5.0f,
			}
		},
		WidgetBlueprint{
			.x = 85, .y = 35, .w = 40, .h = 10,
			.variant = client_count
		},
		WidgetBlueprint{
			.x = 60, .y = 55, .w = 40, .h = 10,
			.variant = TextBlueprint{
				.text = "ADD BOT",
				.wrapped_font_size_in_units = 5.0f,
				.on_action = [](MAYBE const m2::ui::widget::Text& self) -> m2::ui::Action {
					M2_GAME.add_bot();
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 60, .y = 75, .w = 40, .h = 10,
			.variant = TextBlueprint{
				.text = "CANCEL",
				.wrapped_font_size_in_units = 5.0f,
				.on_action = [](MAYBE const Text& self) -> Action {
					M2_GAME.leave_game();
					// TODO kill bots if any
					return make_return_action();
				}
			}
		}
	}
};
