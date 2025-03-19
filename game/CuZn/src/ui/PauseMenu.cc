#include <cuzn/ui/PauseMenu.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <cuzn/ui/Detail.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::widget;
using namespace m2g;
using namespace m2g::pb;

const UiPanelBlueprint pause_menu_blueprint = {
	.name = "PauseMenu",
	.w = 100,
	.h = 100,
	.background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
	.widgets = {
		UiWidgetBlueprint{
			.x = 45, .y = 35, .w = 10, .h = 10,
			.variant = TextBlueprint{
				.text = "RESUME", .onAction = [](MAYBE const widget::Text& self) {
					return MakeReturnAction();  // TODO Return value
				}
			}
		},
		UiWidgetBlueprint{
			.x = 45,
			.y = 55,
			.w = 10,
			.h = 10,
			.variant = TextBlueprint{
				.text = "QUIT",
				.onAction = quit_button_action,
			}
		}
	}
};
