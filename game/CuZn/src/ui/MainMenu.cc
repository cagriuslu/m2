#include <cuzn/ui/MainMenu.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/TextListSelection.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Client.h>
#include <cuzn/ui/Server.h>
#include <m2/Game.h>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

const Blueprint cuzn::main_menu_blueprint = {
	.w = 160,
	.h = 90,
	.border_width_px = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 70, .y = 20, .w = 20, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = TextBlueprint{
				.initial_text = "JOIN", .on_action = [](MAYBE const widget::Text& self) {
					return m2::ui::State::create_execute_sync(&cuzn::ip_port_form);
				}
			}
		},
		WidgetBlueprint{
			.x = 70, .y = 40, .w = 20, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = TextBlueprint{
				.initial_text = "HOST", .on_action = [](MAYBE const widget::Text& self) {
					M2_GAME.host_game(m2::mplayer::Type::TurnBased, 4);
					return m2::ui::State::create_execute_sync(&cuzn::server_lobby);
				}
			}
		},
		WidgetBlueprint{
			.x = 70, .y = 60, .w = 20, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = TextBlueprint{
				.initial_text = "QUIT", .kb_shortcut = SDL_SCANCODE_Q, .on_action = quit_button_action
			}
		}
	}
};
