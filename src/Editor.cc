#include <m2/Editor.h>
#include <m2/Game.hh>
#include <m2/object/Ghost.h>
#include <m2/object/Camera.h>
#include <m2g/SpriteBlueprint.h>

using namespace m2;
using namespace m2::ui;

const Blueprint::Widget::Variant editor_paint_mode_right_hud_selected_sprite = Blueprint::Widget::Image{
	.update_callback = []() -> std::pair<Action,std::optional<m2g::pb::SpriteType>> {
		if (GAME.level->editor_paint_or_place_mode_selected_sprite < 0) {
			return {Action::CONTINUE, {}};
		} else {
			return {Action::CONTINUE, GAME.editor_background_sprites[GAME.level->editor_paint_or_place_mode_selected_sprite]};
		}
	}
};
const Blueprint::Widget::Variant editor_paint_mode_right_hud_left_arrow = Blueprint::Widget::Text{
	.initial_text = "<",
	.action_callback = []() {
		GAME.level->editor_paint_or_place_mode_select_sprite(GAME.level->editor_paint_or_place_mode_selected_sprite - 1);
		return Action::CONTINUE;
	}
};
const Blueprint::Widget::Variant editor_paint_mode_right_hud_right_arrow = Blueprint::Widget::Text{
	.initial_text = ">",
	.action_callback = []() {
		GAME.level->editor_paint_or_place_mode_select_sprite(GAME.level->editor_paint_or_place_mode_selected_sprite + 1);
		return Action::CONTINUE;
	}
};
const Blueprint editor_paint_mode_right_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1,
	.widgets = {
		Blueprint::Widget{
			.x = 4, .y = 4, .w = 11, .h = 11,
			.border_width_px = 1,
			.variant = editor_paint_mode_right_hud_selected_sprite
		},
		Blueprint::Widget{
			.x = 4, .y = 16, .w = 5, .h = 5,
			.border_width_px = 1,
			.variant = editor_paint_mode_right_hud_left_arrow
		},
		Blueprint::Widget{
			.x = 10, .y = 16, .w = 5, .h = 5,
			.border_width_px = 1,
			.variant = editor_paint_mode_right_hud_right_arrow
		}
	}
};

const Blueprint::Widget::Variant editor_place_mode_right_hud_selected_sprite = Blueprint::Widget::Image{
		.update_callback = []() -> std::pair<Action,std::optional<m2g::pb::SpriteType>> {
			if (GAME.level->editor_paint_or_place_mode_selected_sprite < 0) {
				return {Action::CONTINUE, {}};
			} else {
				return {Action::CONTINUE,   std::next(GAME.editor_object_sprites.begin(), GAME.level->editor_paint_or_place_mode_selected_sprite)->second};
			}
		}
};
const Blueprint::Widget::Variant editor_place_mode_right_hud_left_arrow = Blueprint::Widget::Text{
		.initial_text = "<",
		.action_callback = []() {
			GAME.level->editor_paint_or_place_mode_select_sprite(GAME.level->editor_paint_or_place_mode_selected_sprite - 1);
			return Action::CONTINUE;
		}
};
const Blueprint::Widget::Variant editor_place_mode_right_hud_right_arrow = Blueprint::Widget::Text{
		.initial_text = ">",
		.action_callback = []() {
			GAME.level->editor_paint_or_place_mode_select_sprite(GAME.level->editor_paint_or_place_mode_selected_sprite + 1);
			return Action::CONTINUE;
		}
};
const Blueprint editor_place_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				Blueprint::Widget{
						.x = 4, .y = 4, .w = 11, .h = 11,
						.border_width_px = 1,
						.variant = editor_place_mode_right_hud_selected_sprite
				},
				Blueprint::Widget{
						.x = 4, .y = 16, .w = 5, .h = 5,
						.border_width_px = 1,
						.variant = editor_place_mode_right_hud_left_arrow
				},
				Blueprint::Widget{
						.x = 10, .y = 16, .w = 5, .h = 5,
						.border_width_px = 1,
						.variant = editor_place_mode_right_hud_right_arrow
				}
		}
};

const Blueprint::Widget::Variant editor_save_confirmation_text = Blueprint::Widget::Text{
	.initial_text = "Are you sure?"
};
const Blueprint::Widget::Variant editor_save_confirmation_yes_button = Blueprint::Widget::Text{
	.initial_text = "YES",
	.action_callback = []() -> Action {
		// TODO
		return Action::RETURN;
	},
	.kb_shortcut = SDL_SCANCODE_Y
};
const Blueprint::Widget::Variant editor_save_confirmation_no_button = Blueprint::Widget::Text{
		.initial_text = "NO",
		.action_callback = []() -> Action { return Action::RETURN; },
		.kb_shortcut = SDL_SCANCODE_N
};
const Blueprint editor_save_confirmation = {
		.w = 7, .h = 5,
		.border_width_px = 2,
		.widgets = {
				Blueprint::Widget{
					.x = 1, .y = 1, .w = 5, .h = 1,
					.variant = editor_save_confirmation_text
				},
				Blueprint::Widget{
					.x = 1, .y = 3, .w = 2, .h = 1,
					.variant = editor_save_confirmation_yes_button
				},
				Blueprint::Widget{
					.x = 4, .y = 3, .w = 2, .h = 1,
					.variant = editor_save_confirmation_no_button
				}
		}
};

const Blueprint::Widget::Variant editor_left_hud_paint_button = Blueprint::Widget::Text{
	.initial_text = "Paint",
	.action_callback = []() -> Action {
		GAME.level->activate_mode(Level::EditorMode::PAINT);
		GAME.rightHudUIState = State(&editor_paint_mode_right_hud);
		GAME.rightHudUIState->update_positions(GAME.rightHudRect);
		return Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_P
};
const Blueprint::Widget::Variant editor_left_hud_erase_button = Blueprint::Widget::Text{
	.initial_text = "Erase",
	.action_callback = []() -> Action {
		GAME.level->activate_mode(Level::EditorMode::ERASE);
		GAME.rightHudUIState = State(&editor_right_hud);
		GAME.rightHudUIState->update_positions(GAME.rightHudRect);
		return Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_E
};
const Blueprint::Widget::Variant editor_left_hud_place_button = Blueprint::Widget::Text{
	.initial_text = "Place",
	.action_callback = []() -> Action {
		GAME.level->activate_mode(Level::EditorMode::PLACE);
		GAME.rightHudUIState = State(&editor_place_mode_right_hud);
		GAME.rightHudUIState->update_positions(GAME.rightHudRect);
		return Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_O
};
const Blueprint::Widget::Variant editor_left_hud_remove_button = Blueprint::Widget::Text{
	.initial_text = "Remove",
	.action_callback = []() -> Action {
		GAME.level->activate_mode(Level::EditorMode::REMOVE);
		GAME.rightHudUIState = State(&editor_right_hud);
		GAME.rightHudUIState->update_positions(GAME.rightHudRect);
		return Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_R
};
const Blueprint::Widget::Variant editor_left_hud_cancel_button = Blueprint::Widget::Text{
	.initial_text = "Cancel",
	.action_callback = []() -> Action {
		GAME.level->activate_mode(Level::EditorMode::NONE);
		GAME.rightHudUIState = State(&editor_right_hud);
		GAME.rightHudUIState->update_positions(GAME.rightHudRect);
		return Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_C
};
const Blueprint::Widget::Variant editor_left_hud_gridlines_button = Blueprint::Widget::Text{
	.initial_text = "Grid Lines",
	.action_callback = []() -> Action {
		auto* camera = GAME.objects.get(GAME.cameraId);
		if (camera && camera->impl) {
			auto& camera_data = dynamic_cast<m2::obj::Camera&>(*camera->impl);
			camera_data.draw_grid_lines = !camera_data.draw_grid_lines;
		}
		return Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_G
};
const Blueprint::Widget::Variant editor_left_hud_save_button = Blueprint::Widget::Text{
		.initial_text = "Save",
		.action_callback = []() -> Action {
			execute_blocking(&editor_save_confirmation);
			return Action::CONTINUE;
		}
};
const m2::ui::Blueprint m2::ui::editor_left_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1,
	.widgets = {
		Blueprint::Widget{
			.x = 4, .y = 4, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_paint_button
		},
		Blueprint::Widget{
			.x = 4, .y = 8, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_erase_button
		},
		Blueprint::Widget{
			.x = 4, .y = 12, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_place_button
		},
		Blueprint::Widget{
			.x = 4, .y = 16, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_remove_button
		},
		Blueprint::Widget{
			.x = 4, .y = 20, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 1,
			.variant = editor_left_hud_cancel_button
		},
		Blueprint::Widget{
			.x = 4, .y = 60, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_gridlines_button
		},
		Blueprint::Widget{
			.x = 4, .y = 64, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_save_button
		},
	}
};

const Blueprint m2::ui::editor_right_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1
};
