#include <m2/pixel_editor/Ui.h>
#include <m2/Game.h>

using namespace m2;

const ui::Blueprint::Widget::Variant pixel_editor_left_hud_paint_button = ui::Blueprint::Widget::Text{
	.initial_text = "Paint",
	.action_callback = []() -> ui::Action {
		LEVEL.pixel_editor_state->activate_paint_mode();
		return ui::Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_P
};
const ui::Blueprint::Widget::Variant pixel_editor_left_hud_erase_button = ui::Blueprint::Widget::Text{
	.initial_text = "Erase",
	.action_callback = []() -> ui::Action {
		LEVEL.pixel_editor_state->activate_erase_mode();
		return ui::Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_E
};
const ui::Blueprint::Widget::Variant pixel_editor_left_hud_color_picker_button = ui::Blueprint::Widget::Text{
	.initial_text = "Pick",
	.action_callback = []() -> ui::Action {
		LEVEL.pixel_editor_state->activate_color_picker_mode();
		return ui::Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_C
};
const ui::Blueprint::Widget::Variant pixel_editor_left_hud_cancel_button = ui::Blueprint::Widget::Text{
	.initial_text = "Cancel",
	.action_callback = []() -> ui::Action {
		LEVEL.level_editor_state->deactivate_mode();
		return ui::Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_X
};
const ui::Blueprint::Widget::Variant pixel_editor_left_hud_gridlines_button = ui::Blueprint::Widget::Text{
	.initial_text = "Grid",
	.action_callback = []() -> ui::Action {
		LEVEL.toggle_grid();
		return ui::Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_G
};
const ui::Blueprint::Widget::Variant pixel_editor_left_hud_save_button = ui::Blueprint::Widget::Text{
	.initial_text = "Save",
	.action_callback = []() -> ui::Action {
		//execute_blocking(&level_editor_save_confirmation);
		// TODO
		return ui::Action::CONTINUE;
	}
};
const ui::Blueprint m2::ui::pixel_editor_left_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1,
	.widgets = {
		Blueprint::Widget{
			.x = 4, .y = 4, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 2,
			.variant = pixel_editor_left_hud_paint_button
		},
		Blueprint::Widget{
			.x = 4, .y = 8, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 2,
			.variant = pixel_editor_left_hud_erase_button
		},
		Blueprint::Widget{
			.x = 4, .y = 12, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 2,
			.variant = pixel_editor_left_hud_color_picker_button
		},
		Blueprint::Widget{
			.x = 4, .y = 16, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 2,
			.variant = pixel_editor_left_hud_cancel_button
		},
		Blueprint::Widget{
			.x = 4, .y = 60, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 2,
			.variant = pixel_editor_left_hud_gridlines_button
		},
		Blueprint::Widget{
			.x = 4, .y = 64, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 2,
			.variant = pixel_editor_left_hud_save_button
		},
	}
};

const ui::Blueprint::Widget::Variant pixel_editor_right_hud_red_selection = ui::Blueprint::Widget::IntegerSelection{
		.min_value = 0,
		.max_value = 255,
		.initial_value = 0,
		.update_callback = []() -> std::optional<int> {
			return (int) LEVEL.pixel_editor_state->selected_color.r;
		},
		.action_callback = [](int selection) -> ui::Action {
			LEVEL.pixel_editor_state->select_color(SDL_Color{
				.r = (uint8_t) selection,
				.g = LEVEL.pixel_editor_state->selected_color.g,
				.b = LEVEL.pixel_editor_state->selected_color.b,
				.a = LEVEL.pixel_editor_state->selected_color.a});
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant pixel_editor_right_hud_green_selection = ui::Blueprint::Widget::IntegerSelection{
		.min_value = 0,
		.max_value = 255,
		.initial_value = 0,
		.update_callback = []() -> std::optional<int> {
			return (int) LEVEL.pixel_editor_state->selected_color.g;
		},
		.action_callback = [](int selection) -> ui::Action {
			LEVEL.pixel_editor_state->select_color(SDL_Color{
				.r = LEVEL.pixel_editor_state->selected_color.r,
				.g = (uint8_t) selection,
				.b = LEVEL.pixel_editor_state->selected_color.b,
				.a = LEVEL.pixel_editor_state->selected_color.a});
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant pixel_editor_right_hud_blue_selection = ui::Blueprint::Widget::IntegerSelection{
		.min_value = 0,
		.max_value = 255,
		.initial_value = 0,
		.update_callback = []() -> std::optional<int> {
			return (int) LEVEL.pixel_editor_state->selected_color.b;
		},
		.action_callback = [](int selection) -> ui::Action {
			LEVEL.pixel_editor_state->select_color(SDL_Color{
				.r = LEVEL.pixel_editor_state->selected_color.r,
				.g = LEVEL.pixel_editor_state->selected_color.g,
				.b = (uint8_t) selection,
				.a = LEVEL.pixel_editor_state->selected_color.a});
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant pixel_editor_right_hud_alpha_selection = ui::Blueprint::Widget::IntegerSelection{
		.min_value = 0,
		.max_value = 255,
		.initial_value = 0,
		.update_callback = []() -> std::optional<int> {
			return (int) LEVEL.pixel_editor_state->selected_color.a;
		},
		.action_callback = [](int selection) -> ui::Action {
			LEVEL.pixel_editor_state->select_color(SDL_Color{
				.r = LEVEL.pixel_editor_state->selected_color.r,
				.g = LEVEL.pixel_editor_state->selected_color.g,
				.b = LEVEL.pixel_editor_state->selected_color.b,
				.a = (uint8_t) selection});
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint m2::ui::pixel_editor_right_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1,
	.widgets = {
		Blueprint::Widget{
			.x = 4, .y = 4, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 2,
			.variant = pixel_editor_right_hud_red_selection
		},
		Blueprint::Widget{
			.x = 4, .y = 8, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 2,
			.variant = pixel_editor_right_hud_green_selection
		},
		Blueprint::Widget{
			.x = 4, .y = 12, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 2,
			.variant = pixel_editor_right_hud_blue_selection
		},
		Blueprint::Widget{
			.x = 4, .y = 16, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 2,
			.variant = pixel_editor_right_hud_alpha_selection
		},
	}
};
