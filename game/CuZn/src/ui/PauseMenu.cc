#include <cuzn/ui/PauseMenu.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <cuzn/ui/Detail.h>
#include <m2/Game.h>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

const PanelBlueprint pause_menu_blueprint = {
	.w = 100,
	.h = 100,
	.background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
	.widgets = {
		WidgetBlueprint{
			.x = 45, .y = 35, .w = 10, .h = 10,
			.variant = TextBlueprint{
				.text = "RESUME", .on_action = [](MAYBE const widget::Text& self) {
					return MakeReturnAction();  // TODO Return value
				}
			}
		},
		WidgetBlueprint{
			.x = 45,
			.y = 55,
			.w = 10,
			.h = 10,
			.variant = TextBlueprint{
				.text = "QUIT",
				.kb_shortcut = SDL_SCANCODE_Q,
				.on_action = quit_button_action,
			}
		}
	}
};
