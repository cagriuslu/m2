#include <m2/Editor.h>
#include <m2/Game.hh>
#include <m2/object/Ghost.h>
#include <m2/object/Camera.h>
#include <m2g/SpriteBlueprint.h>

using namespace m2;
using namespace m2::ui;

Blueprint::Widget::Variant editor_paint_mode_image_selection = Blueprint::Widget::ImageSelection{
	.action_callback = [](m2g::pb::SpriteType selection) -> Action {
		GAME.level->editor_paint_mode_select_sprite_type(selection);
		return Action::CONTINUE;
	}
};
Blueprint editor_paint_mode_right_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1,
	.widgets = {
		Blueprint::Widget{
			.x = 4, .y = 4, .w = 11, .h = 14,
			.border_width_px = 1,
			.variant = editor_paint_mode_image_selection
		}
	}
};

Blueprint::Widget::Variant editor_place_mode_right_hud_object_type_selection = Blueprint::Widget::TextSelection{
	.action_callback = [](const std::string& selection) -> Action {
		auto object_type = m2g::pb::ObjectType::NO_OBJECT;
		m2g::pb::ObjectType_Parse(selection, &object_type);
		GAME.level->editor_place_mode_select_object_type(object_type);
		return Action::CONTINUE;
	}
};
Blueprint editor_place_mode_right_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1,
	.widgets = {
		Blueprint::Widget{
			.x = 4, .y = 4, .w = 11, .h = 4,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = editor_place_mode_right_hud_object_type_selection
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

		// Fill tile selector with editor-enabled sprites
		std::for_each(editor_paint_mode_right_hud.widgets.begin(), editor_paint_mode_right_hud.widgets.end(), [](auto &widget) {
			std::visit(m2::overloaded{
				[](Blueprint::Widget::ImageSelection &v) {
					if (v.list.empty()) {
						std::copy(std::begin(GAME.editor_background_sprites), std::end(GAME.editor_background_sprites), std::back_inserter(v.list));
					}
				},
				[](MAYBE auto &v) {}
			}, widget.variant);
		});

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

		// Fill object type selector with editor-enabled object types
		std::for_each(editor_place_mode_right_hud.widgets.begin(), editor_place_mode_right_hud.widgets.end(), [](auto& widget) {
			std::visit(m2::overloaded {
				[](Blueprint::Widget::TextSelection& v) {
					if (v.list.empty()) {
						for (auto& [obj_type, spt] : GAME.editor_object_sprites) {
							v.list.emplace_back(m2g::pb::ObjectType_Name(obj_type));
						}
					}
				},
				[](MAYBE auto& v) {}
			}, widget.variant);
		});

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
