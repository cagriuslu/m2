#include <m2/pixel_editor/Ui.h>
#include <m2/Game.h>
#include <m2/ui/widget/IntegerInput.h>

using namespace m2;
using namespace m2::ui;

const widget::TextBlueprint pixel_editor_left_hud_paint_button = {
		.text = "Paint",
		.kb_shortcut = SDL_SCANCODE_P,
		.on_action = [](MAYBE const widget::Text &self) -> Action {
			std::get<pedit::State>(M2_LEVEL.type_state).activate_paint_mode();
			return make_continue_action();
		}
};
const widget::TextBlueprint pixel_editor_left_hud_erase_button = {
		.text = "Erase",
		.kb_shortcut = SDL_SCANCODE_E,
		.on_action = [](MAYBE const widget::Text &self) -> Action {
			std::get<pedit::State>(M2_LEVEL.type_state).activate_erase_mode();
			return make_continue_action();
		}
};
const widget::TextBlueprint pixel_editor_left_hud_color_picker_button = {
		.text = "Pick",
		.kb_shortcut = SDL_SCANCODE_C,
		.on_action = [](MAYBE const widget::Text &self) -> Action {
			std::get<pedit::State>(M2_LEVEL.type_state).activate_color_picker_mode();
			return make_continue_action();
		}
};
const widget::TextBlueprint pixel_editor_left_hud_cancel_button = {
		.text = "Cancel",
		.kb_shortcut = SDL_SCANCODE_X,
		.on_action = [](MAYBE const widget::Text &self) -> Action {
			std::get<pedit::State>(M2_LEVEL.type_state).deactivate_mode();
			return make_continue_action();
		}
};
const widget::TextBlueprint pixel_editor_left_hud_gridlines_button = {
		.text = "Grid",
		.kb_shortcut = SDL_SCANCODE_G,
		.on_action = [](MAYBE const widget::Text &self) -> Action {
			if (M2_LEVEL.dynamic_grid_lines_loader) {
				M2_LEVEL.dynamic_grid_lines_loader.reset();
			} else {
				M2_LEVEL.dynamic_grid_lines_loader.emplace(SDL_Color{127, 127, 255, 127});
			}
			return make_continue_action();
		}
};
const widget::TextBlueprint pixel_editor_left_hud_save_button = {
		.text = "Save",
		.on_action = [](MAYBE const widget::Text &self) -> Action {
			//execute_blocking(&level_editor_save_confirmation);
			// TODO
			return make_continue_action();
		}
};
const Blueprint m2::ui::pixel_editor_left_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 4, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = pixel_editor_left_hud_paint_button
				},
				WidgetBlueprint{
						.x = 4, .y = 8, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = pixel_editor_left_hud_erase_button
				},
				WidgetBlueprint{
						.x = 4, .y = 12, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = pixel_editor_left_hud_color_picker_button
				},
				WidgetBlueprint{
						.x = 4, .y = 16, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = pixel_editor_left_hud_cancel_button
				},
				WidgetBlueprint{
						.x = 4, .y = 60, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = pixel_editor_left_hud_gridlines_button
				},
				WidgetBlueprint{
						.x = 4, .y = 64, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = pixel_editor_left_hud_save_button
				},
		}
};

const widget::IntegerInputBlueprint pixel_editor_right_hud_red_selection = {
		.min_value = 0,
		.max_value = 255,
		.initial_value = 0,
		.on_update = [](MAYBE const widget::IntegerInput& self) -> std::optional<int> {
			return (int) std::get<pedit::State>(M2_LEVEL.type_state).selected_color.r;
		},
		.on_action = [](const widget::IntegerInput& self) -> Action {
			std::get<pedit::State>(M2_LEVEL.type_state).select_color(SDL_Color{
					.r = (uint8_t) self.value(),
					.g = std::get<pedit::State>(M2_LEVEL.type_state).selected_color.g,
					.b = std::get<pedit::State>(M2_LEVEL.type_state).selected_color.b,
					.a = std::get<pedit::State>(M2_LEVEL.type_state).selected_color.a});
			return make_continue_action();
		}
};
const widget::IntegerInputBlueprint pixel_editor_right_hud_green_selection = {
		.min_value = 0,
		.max_value = 255,
		.initial_value = 0,
		.on_update = [](MAYBE const widget::IntegerInput& self) -> std::optional<int> {
			return (int) std::get<pedit::State>(M2_LEVEL.type_state).selected_color.g;
		},
		.on_action = [](const widget::IntegerInput& self) -> Action {
			std::get<pedit::State>(M2_LEVEL.type_state).select_color(SDL_Color{
					.r = std::get<pedit::State>(M2_LEVEL.type_state).selected_color.r,
					.g = (uint8_t) self.value(),
					.b = std::get<pedit::State>(M2_LEVEL.type_state).selected_color.b,
					.a = std::get<pedit::State>(M2_LEVEL.type_state).selected_color.a});
			return make_continue_action();
		}
};
const widget::IntegerInputBlueprint pixel_editor_right_hud_blue_selection = {
		.min_value = 0,
		.max_value = 255,
		.initial_value = 0,
		.on_update = [](MAYBE const widget::IntegerInput& self) -> std::optional<int> {
			return (int) std::get<pedit::State>(M2_LEVEL.type_state).selected_color.b;
		},
		.on_action = [](const widget::IntegerInput& self) -> Action {
			std::get<pedit::State>(M2_LEVEL.type_state).select_color(SDL_Color{
					.r = std::get<pedit::State>(M2_LEVEL.type_state).selected_color.r,
					.g = std::get<pedit::State>(M2_LEVEL.type_state).selected_color.g,
					.b = (uint8_t) self.value(),
					.a = std::get<pedit::State>(M2_LEVEL.type_state).selected_color.a});
			return make_continue_action();
		}
};
const widget::IntegerInputBlueprint pixel_editor_right_hud_alpha_selection = {
		.min_value = 0,
		.max_value = 255,
		.initial_value = 0,
		.on_update = [](MAYBE const widget::IntegerInput& self) -> std::optional<int> {
			return (int) std::get<pedit::State>(M2_LEVEL.type_state).selected_color.a;
		},
		.on_action = [](const widget::IntegerInput& self) -> Action {
			std::get<pedit::State>(M2_LEVEL.type_state).select_color(SDL_Color{
					.r = std::get<pedit::State>(M2_LEVEL.type_state).selected_color.r,
					.g = std::get<pedit::State>(M2_LEVEL.type_state).selected_color.g,
					.b = std::get<pedit::State>(M2_LEVEL.type_state).selected_color.b,
					.a = (uint8_t) self.value()});
			return make_continue_action();
		}
};
const Blueprint m2::ui::pixel_editor_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 4, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = pixel_editor_right_hud_red_selection
				},
				WidgetBlueprint{
						.x = 4, .y = 8, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = pixel_editor_right_hud_green_selection
				},
				WidgetBlueprint{
						.x = 4, .y = 12, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = pixel_editor_right_hud_blue_selection
				},
				WidgetBlueprint{
						.x = 4, .y = 16, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = pixel_editor_right_hud_alpha_selection
				},
		}
};
