#include <m2/ui/MessageBox.h>
#include <m2/math/RectF.h>

const m2::UiPanelBlueprint m2::DefaultMessageBoxBlueprint = {
	.name = "MessageBox",
	.w = 1, .h = 1,
	.border_width = 0,
	.ignore_events = true,
	.widgets = {UiWidgetBlueprint{
		.name = "MessageText",
		.x = 0, .y = 0, .w = 1, .h = 1,
		.border_width = 0,
		.background_color = SDL_Color{127, 127, 127, 127},
		.variant =
			widget::TextBlueprint{
				.text = "Placeholder text",
				.horizontal_alignment = TextHorizontalAlignment::LEFT,
				.wrapped_font_size_in_units = 0.85f
			}
	}}
};

const m2::RectF m2::DefaultMessageBoxArea = {
	1.0f / 6.0f,
	1.0f - 1.0f / 25.0f,
	2.0f / 3.0f,
	1.0f / 25.0f
};
