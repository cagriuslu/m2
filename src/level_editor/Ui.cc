#include <m2/level_editor/Ui.h>
#include "m2/Game.h"
#include "m2/object/Ghost.h"
#include "m2/object/Camera.h"

using namespace m2;

ui::Blueprint::Widget::Variant level_editor_paint_mode_image_selection = ui::Blueprint::Widget::ImageSelection{
		.action_callback = [](m2g::pb::SpriteType selection) -> ui::Action {
			std::get<Level::LevelEditorState::PaintMode>(LEVEL.level_editor_state->mode).select_sprite_type(selection);
			return ui::Action::CONTINUE;
		}
};
ui::Blueprint level_editor_paint_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				ui::Blueprint::Widget{
						.x = 4, .y = 4, .w = 11, .h = 14,
						.border_width_px = 1,
						.variant = level_editor_paint_mode_image_selection
				}
		}
};

ui::Blueprint::Widget::Variant level_editor_place_mode_right_hud_object_type_selection = ui::Blueprint::Widget::TextSelection{
		.action_callback = [](const std::string& selection) -> ui::Action {
			auto object_type = m2g::pb::ObjectType::NO_OBJECT;
			m2g::pb::ObjectType_Parse(selection, &object_type);
			std::get<Level::LevelEditorState::PlaceMode>(LEVEL.level_editor_state->mode).select_object_type(object_type);
			return ui::Action::CONTINUE;
		}
};
ui::Blueprint::Widget::Variant level_editor_place_mode_right_hud_group_type_selection = ui::Blueprint::Widget::TextSelection{
		.action_callback = [](const std::string &selection) -> ui::Action {
			auto group_type = m2g::pb::GroupType::NO_GROUP;
			m2g::pb::GroupType_Parse(selection, &group_type);
			std::get<Level::LevelEditorState::PlaceMode>(LEVEL.level_editor_state->mode).select_group_type(group_type);
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant level_editor_place_mode_right_hud_group_instance_selection = ui::Blueprint::Widget::IntegerSelection{
		.min_value = 0,
		.max_value = 999,
		.initial_value = 0,
		.action_callback = [](int selection) -> ui::Action {
			std::get<Level::LevelEditorState::PlaceMode>(LEVEL.level_editor_state->mode).select_group_instance(selection);
			return ui::Action::CONTINUE;
		}
};
ui::Blueprint level_editor_place_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				ui::Blueprint::Widget{
						.x = 4, .y = 4, .w = 11, .h = 4,
						.border_width_px = 1,
						.padding_width_px = 5,
						.variant = level_editor_place_mode_right_hud_object_type_selection
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 9, .w = 11, .h = 4,
						.border_width_px = 1,
						.padding_width_px = 5,
						.variant = level_editor_place_mode_right_hud_group_type_selection
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 14, .w = 11, .h = 4,
						.border_width_px = 1,
						.padding_width_px = 5,
						.variant = level_editor_place_mode_right_hud_group_instance_selection
				}
		}
};

ui::Blueprint::Widget::Variant level_editor_shift_mode_right_hud_shift_direction_selection = ui::Blueprint::Widget::TextSelection{
		.list = {"Right", "Down", "Right & Down"},
		.action_callback = [](const std::string& selection) -> ui::Action {
			if (selection == "Right") {
				std::get<Level::LevelEditorState::ShiftMode>(LEVEL.level_editor_state->mode).shift_type = Level::LevelEditorState::ShiftMode::ShiftType::RIGHT;
			} else if (selection == "Down") {
				std::get<Level::LevelEditorState::ShiftMode>(LEVEL.level_editor_state->mode).shift_type = Level::LevelEditorState::ShiftMode::ShiftType::DOWN;
			} else if (selection == "Right & Down") {
				std::get<Level::LevelEditorState::ShiftMode>(LEVEL.level_editor_state->mode).shift_type = Level::LevelEditorState::ShiftMode::ShiftType::RIGHT_N_DOWN;
			}
			return ui::Action::CONTINUE;
		}
};
ui::Blueprint level_editor_shift_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				ui::Blueprint::Widget{
						.x = 4, .y = 4, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = level_editor_shift_mode_right_hud_shift_direction_selection
				}
		}
};

const ui::Blueprint::Widget::Variant level_editor_save_confirmation_text = ui::Blueprint::Widget::Text{
		.initial_text = "Are you sure?"
};
const ui::Blueprint::Widget::Variant level_editor_save_confirmation_yes_button = ui::Blueprint::Widget::Text{
		.initial_text = "YES",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->save();
			return ui::Action::RETURN;
		},
		.kb_shortcut = SDL_SCANCODE_Y
};
const ui::Blueprint::Widget::Variant level_editor_save_confirmation_no_button = ui::Blueprint::Widget::Text{
		.initial_text = "NO",
		.action_callback = []() -> ui::Action { return ui::Action::RETURN; },
		.kb_shortcut = SDL_SCANCODE_N
};
const ui::Blueprint level_editor_save_confirmation = {
		.w = 7, .h = 5,
		.border_width_px = 2,
		.widgets = {
				ui::Blueprint::Widget{
						.x = 1, .y = 1, .w = 5, .h = 1,
						.variant = level_editor_save_confirmation_text
				},
				ui::Blueprint::Widget{
						.x = 1, .y = 3, .w = 2, .h = 1,
						.variant = level_editor_save_confirmation_yes_button
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 3, .w = 2, .h = 1,
						.variant = level_editor_save_confirmation_no_button
				}
		}
};

const ui::Blueprint::Widget::Variant level_editor_left_hud_paint_button = ui::Blueprint::Widget::Text{
		.initial_text = "Paint",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->activate_paint_mode();
			// Fill tile selector with editor-enabled sprites
			std::for_each(level_editor_paint_mode_right_hud.widgets.begin(), level_editor_paint_mode_right_hud.widgets.end(), [](auto &widget) {
				std::visit(m2::overloaded{
						[](ui::Blueprint::Widget::ImageSelection &v) {
							if (v.list.empty()) {
								std::copy(std::begin(GAME.level_editor_background_sprites), std::end(GAME.level_editor_background_sprites), std::back_inserter(v.list));
							}
						},
						[](MAYBE auto &v) {}
				}, widget.variant);
			});

			LEVEL.rightHudUIState = ui::State(&level_editor_paint_mode_right_hud);
			LEVEL.rightHudUIState->update_positions(GAME.rightHudRect);
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_P
};
const ui::Blueprint::Widget::Variant level_editor_left_hud_erase_button = ui::Blueprint::Widget::Text{
		.initial_text = "Erase",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->activate_erase_mode();
			LEVEL.rightHudUIState = ui::State(&level_editor::ui::right_hud);
			LEVEL.rightHudUIState->update_positions(GAME.rightHudRect);
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_E
};
const ui::Blueprint::Widget::Variant level_editor_left_hud_place_button = ui::Blueprint::Widget::Text{
		.initial_text = "Place",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->activate_place_mode();
			// Fill object type selector with editor-enabled object types
			auto& object_type_selection = std::get<ui::Blueprint::Widget::TextSelection>(level_editor_place_mode_right_hud.widgets[0].variant);
			if (object_type_selection.list.empty()) {
				for (auto& [obj_type, spt] : GAME.level_editor_object_sprites) {
					object_type_selection.list.emplace_back(m2g::pb::ObjectType_Name(obj_type));
				}
			}
			// Fill group type selector
			auto& group_type_selection = std::get<ui::Blueprint::Widget::TextSelection>(level_editor_place_mode_right_hud.widgets[1].variant);
			if (group_type_selection.list.empty()) {
				for (int e  = 0; e < proto::enum_value_count<m2g::pb::GroupType>(); ++e) {
					group_type_selection.list.emplace_back(proto::enum_name<m2g::pb::GroupType>(e));
				}
			}

			LEVEL.rightHudUIState = ui::State(&level_editor_place_mode_right_hud);
			LEVEL.rightHudUIState->update_positions(GAME.rightHudRect);
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_O
};
const ui::Blueprint::Widget::Variant level_editor_left_hud_remove_button = ui::Blueprint::Widget::Text{
		.initial_text = "Remove",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->activate_remove_mode();
			LEVEL.rightHudUIState = ui::State(&level_editor::ui::right_hud);
			LEVEL.rightHudUIState->update_positions(GAME.rightHudRect);
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_R
};
const ui::Blueprint::Widget::Variant level_editor_left_hud_shift_button = ui::Blueprint::Widget::Text{
		.initial_text = "Shift",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->activate_shift_mode();
			LEVEL.rightHudUIState = ui::State(&level_editor_shift_mode_right_hud);
			LEVEL.rightHudUIState->update_positions(GAME.rightHudRect);
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_X
};
const ui::Blueprint::Widget::Variant level_editor_left_hud_cancel_button = ui::Blueprint::Widget::Text{
		.initial_text = "Cancel",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->deactivate_mode();
			LEVEL.rightHudUIState = ui::State(&level_editor::ui::right_hud);
			LEVEL.rightHudUIState->update_positions(GAME.rightHudRect);
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_X
};
const ui::Blueprint::Widget::Variant level_editor_left_hud_gridlines_button = ui::Blueprint::Widget::Text{
		.initial_text = "Grid",
		.action_callback = []() -> ui::Action {
			LEVEL.toggle_grid();
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_G
};
const ui::Blueprint::Widget::Variant level_editor_left_hud_save_button = ui::Blueprint::Widget::Text{
		.initial_text = "Save",
		.action_callback = []() -> ui::Action {
			execute_blocking(&level_editor_save_confirmation);
			return ui::Action::CONTINUE;
		}
};
const m2::ui::Blueprint m2::level_editor::ui::left_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 4, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 4,
						.variant = level_editor_left_hud_paint_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 8, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 4,
						.variant = level_editor_left_hud_erase_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 12, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 4,
						.variant = level_editor_left_hud_place_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 16, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 4,
						.variant = level_editor_left_hud_remove_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 20, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 1,
						.variant = level_editor_left_hud_shift_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 24, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 1,
						.variant = level_editor_left_hud_cancel_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 60, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 4,
						.variant = level_editor_left_hud_gridlines_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 64, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 4,
						.variant = level_editor_left_hud_save_button
				},
		}
};

const ui::Blueprint m2::level_editor::ui::right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1
};
