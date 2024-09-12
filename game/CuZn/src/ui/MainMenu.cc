#include <cuzn/ui/MainMenu.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Client.h>
#include <cuzn/ui/Server.h>
#include <m2/Game.h>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

const PanelBlueprint main_menu_blueprint = {
	.w = 160,
	.h = 90,
	.border_width = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 70, .y = 20, .w = 20, .h = 10,
			.border_width = 1.0f,
			.variant = TextBlueprint{
				.text = "JOIN", .on_action = [](MAYBE const widget::Text& self) {
					return m2::ui::Panel::create_execute_sync(&ip_port_form);
				}
			}
		},
		WidgetBlueprint{
			.x = 70, .y = 40, .w = 20, .h = 10,
			.border_width = 1.0f,
			.variant = TextBlueprint{
				.text = "HOST", .on_action = [](MAYBE const widget::Text& self) {
					M2_GAME.host_game(m2::mplayer::Type::TurnBased, 4);
					return m2::ui::Panel::create_execute_sync(&server_lobby);
				}
			}
		},
		WidgetBlueprint{
			.x = 70, .y = 60, .w = 20, .h = 10,
			.border_width = 1.0f,
			.variant = TextBlueprint{
				.text = "QUIT", .kb_shortcut = SDL_SCANCODE_Q, .on_action = quit_button_action
			}
		}
	}
};
