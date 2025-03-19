#include <cuzn/ui/MainMenu.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Client.h>
#include <cuzn/ui/Server.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::widget;
using namespace m2g;
using namespace m2g::pb;

const UiPanelBlueprint main_menu_blueprint = {
	.name = "MainMenu",
	.w = 160,
	.h = 90,
	.border_width = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		UiWidgetBlueprint{
			.x = 70, .y = 20, .w = 20, .h = 10,
			.variant = TextBlueprint{
				.text = "JOIN",
				.wrapped_font_size_in_units = 5.0f,
				.onAction = [](MAYBE const widget::Text& self) {
					if (auto action = m2::UiPanel::create_and_run_blocking(&ip_port_form); action.IsReturn()) {
						// If the sub menu has returned, stay at main menu
						return MakeContinueAction();
					} else {
						// If the sub menu has returned something else (clear stack, quit), propagate
						return action;
					}
				}
			}
		},
		UiWidgetBlueprint{
			.x = 70, .y = 40, .w = 20, .h = 10,
			.variant = TextBlueprint{
				.text = "HOST",
				.wrapped_font_size_in_units = 5.0f,
				.onAction = [](MAYBE const widget::Text& self) {
					M2_GAME.HostGame(m2::mplayer::Type::TurnBased, 4);
					if (auto action = m2::UiPanel::create_and_run_blocking(&server_lobby); action.IsReturn()) {
						// If the sub menu has returned, stay at main menu
						return MakeContinueAction();
					} else {
						// If the sub menu has returned something else (clear stack, quit), propagate
						return action;
					}
				}
			}
		},
		UiWidgetBlueprint{
			.x = 70, .y = 60, .w = 20, .h = 10,
			.variant = TextBlueprint{
				.text = "QUIT",
				.wrapped_font_size_in_units = 5.0f,
				.onAction = quit_button_action
			}
		}
	}
};
