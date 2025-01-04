#include <cuzn/ui/Notification.h>

using namespace m2;

UiPanelBlueprint generate_notification_panel_blueprint(const std::string& msg) {
	return UiPanelBlueprint{
		.w = 1, .h = 1,
		.border_width = 0,
		.ignore_events = true,
		.timeout_s = 8.0f,
		.widgets = {
			UiWidgetBlueprint{
				.x = 0, .y = 0, .w = 1, .h = 1,
				.border_width = 0,
				.background_color = SDL_Color{127, 127, 127, 127},
				.variant = widget::TextBlueprint{
					.text = msg,
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.9f,
					.kb_shortcut = SDL_SCANCODE_RETURN,
				}
			}
		}
	};
}
