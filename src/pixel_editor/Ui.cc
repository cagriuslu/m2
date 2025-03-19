#include <m2/pixel_editor/Ui.h>
#include <m2/Game.h>
#include <m2/ui/widget/IntegerInput.h>

using namespace m2;
using namespace m2;

const widget::TextBlueprint pixel_editor_left_hud_paint_button = {
		.text = "Paint",
		.onAction = [](MAYBE const widget::Text &self) -> UiAction {
			std::get<pixel_editor::State>(M2_LEVEL.stateVariant).activate_paint_mode();
			return MakeContinueAction();
		}
};
const widget::TextBlueprint pixel_editor_left_hud_erase_button = {
		.text = "Erase",
		.onAction = [](MAYBE const widget::Text &self) -> UiAction {
			std::get<pixel_editor::State>(M2_LEVEL.stateVariant).activate_erase_mode();
			return MakeContinueAction();
		}
};
const widget::TextBlueprint pixel_editor_left_hud_color_picker_button = {
		.text = "Pick",
		.onAction = [](MAYBE const widget::Text &self) -> UiAction {
			std::get<pixel_editor::State>(M2_LEVEL.stateVariant).activate_color_picker_mode();
			return MakeContinueAction();
		}
};
const widget::TextBlueprint pixel_editor_left_hud_cancel_button = {
		.text = "Cancel",
		.onAction = [](MAYBE const widget::Text &self) -> UiAction {
			std::get<pixel_editor::State>(M2_LEVEL.stateVariant).deactivate_mode();
			return MakeContinueAction();
		}
};
const widget::TextBlueprint pixel_editor_left_hud_gridlines_button = {
		.text = "Grid",
		.onAction = [](MAYBE const widget::Text &self) -> UiAction {
			return MakeContinueAction();
		}
};
const widget::TextBlueprint pixel_editor_left_hud_save_button = {
		.text = "Save",
		.onAction = [](MAYBE const widget::Text &self) -> UiAction {
			//execute_blocking(&level_editor_save_confirmation);
			// TODO
			return MakeContinueAction();
		}
};
const UiPanelBlueprint m2::pixel_editor_left_hud = {
	.name = "LeftHud",
		.w = 19, .h = 72,
		.widgets = {
				UiWidgetBlueprint{
						.x = 4, .y = 4, .w = 11, .h = 3,
						.variant = pixel_editor_left_hud_paint_button
				},
				UiWidgetBlueprint{
						.x = 4, .y = 8, .w = 11, .h = 3,
						.variant = pixel_editor_left_hud_erase_button
				},
				UiWidgetBlueprint{
						.x = 4, .y = 12, .w = 11, .h = 3,
						.variant = pixel_editor_left_hud_color_picker_button
				},
				UiWidgetBlueprint{
						.x = 4, .y = 16, .w = 11, .h = 3,
						.variant = pixel_editor_left_hud_cancel_button
				},
				UiWidgetBlueprint{
						.x = 4, .y = 60, .w = 11, .h = 3,
						.variant = pixel_editor_left_hud_gridlines_button
				},
				UiWidgetBlueprint{
						.x = 4, .y = 64, .w = 11, .h = 3,
						.variant = pixel_editor_left_hud_save_button
				},
		}
};

const widget::IntegerInputBlueprint pixel_editor_right_hud_red_selection = {
		.min_value = 0,
		.max_value = 255,
		.initial_value = 0,
		.onUpdate = [](MAYBE const widget::IntegerInput& self) -> std::optional<int> {
			return (int) std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.r;
		},
		.onAction = [](const widget::IntegerInput& self) -> UiAction {
			std::get<pixel_editor::State>(M2_LEVEL.stateVariant).select_color(SDL_Color{
					.r = (uint8_t) self.value(),
					.g = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.g,
					.b = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.b,
					.a = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.a});
			return MakeContinueAction();
		}
};
const widget::IntegerInputBlueprint pixel_editor_right_hud_green_selection = {
		.min_value = 0,
		.max_value = 255,
		.initial_value = 0,
		.onUpdate = [](MAYBE const widget::IntegerInput& self) -> std::optional<int> {
			return (int) std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.g;
		},
		.onAction = [](const widget::IntegerInput& self) -> UiAction {
			std::get<pixel_editor::State>(M2_LEVEL.stateVariant).select_color(SDL_Color{
					.r = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.r,
					.g = (uint8_t) self.value(),
					.b = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.b,
					.a = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.a});
			return MakeContinueAction();
		}
};
const widget::IntegerInputBlueprint pixel_editor_right_hud_blue_selection = {
		.min_value = 0,
		.max_value = 255,
		.initial_value = 0,
		.onUpdate = [](MAYBE const widget::IntegerInput& self) -> std::optional<int> {
			return (int) std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.b;
		},
		.onAction = [](const widget::IntegerInput& self) -> UiAction {
			std::get<pixel_editor::State>(M2_LEVEL.stateVariant).select_color(SDL_Color{
					.r = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.r,
					.g = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.g,
					.b = (uint8_t) self.value(),
					.a = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.a});
			return MakeContinueAction();
		}
};
const widget::IntegerInputBlueprint pixel_editor_right_hud_alpha_selection = {
		.min_value = 0,
		.max_value = 255,
		.initial_value = 0,
		.onUpdate = [](MAYBE const widget::IntegerInput& self) -> std::optional<int> {
			return (int) std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.a;
		},
		.onAction = [](const widget::IntegerInput& self) -> UiAction {
			std::get<pixel_editor::State>(M2_LEVEL.stateVariant).select_color(SDL_Color{
					.r = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.r,
					.g = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.g,
					.b = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color.b,
					.a = (uint8_t) self.value()});
			return MakeContinueAction();
		}
};
const UiPanelBlueprint m2::pixel_editor_right_hud = {
	.name = "RightHud",
		.w = 19, .h = 72,
		.widgets = {
				UiWidgetBlueprint{
						.x = 4, .y = 4, .w = 11, .h = 3,
						.variant = pixel_editor_right_hud_red_selection
				},
				UiWidgetBlueprint{
						.x = 4, .y = 8, .w = 11, .h = 3,
						.variant = pixel_editor_right_hud_green_selection
				},
				UiWidgetBlueprint{
						.x = 4, .y = 12, .w = 11, .h = 3,
						.variant = pixel_editor_right_hud_blue_selection
				},
				UiWidgetBlueprint{
						.x = 4, .y = 16, .w = 11, .h = 3,
						.variant = pixel_editor_right_hud_alpha_selection
				},
		}
};
