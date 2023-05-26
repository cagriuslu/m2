#include <m2/level_editor/Ui.h>
#include "m2/Game.h"
#include "m2/object/Ghost.h"

using namespace m2;

const ui::Blueprint::Widget::Variant paint_mode_title = ui::Blueprint::Widget::Text{
	.initial_text = "PAINT"
};
const ui::Blueprint::Widget::Variant paint_mode_image_selection = ui::Blueprint::Widget::ImageSelection{
		.action_callback = [](m2g::pb::SpriteType selection) -> ui::Action {
			std::get<ledit::State::PaintMode>(LEVEL.level_editor_state->mode).select_sprite_type(selection);
			return ui::Action::CONTINUE;
		}
};
ui::Blueprint paint_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				ui::Blueprint::Widget{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = paint_mode_title
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 6, .w = 11, .h = 14,
						.border_width_px = 1,
						.variant = paint_mode_image_selection
				}
		}
};

const ui::Blueprint::Widget::Variant erase_mode_title = ui::Blueprint::Widget::Text{
		.initial_text = "ERASE"
};
const ui::Blueprint erase_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				ui::Blueprint::Widget{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = erase_mode_title
				}
		}
};

const ui::Blueprint::Widget::Variant place_mode_title = ui::Blueprint::Widget::Text{
		.initial_text = "PLACE"
};
const ui::Blueprint::Widget::Variant place_mode_right_hud_object_type_selection = ui::Blueprint::Widget::TextSelection{
		.action_callback = [](const std::string& selection) -> ui::Action {
			auto object_type = m2g::pb::ObjectType::NO_OBJECT;
			m2g::pb::ObjectType_Parse(selection, &object_type);
			std::get<ledit::State::PlaceMode>(LEVEL.level_editor_state->mode).select_object_type(object_type);
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant place_mode_right_hud_group_type_selection = ui::Blueprint::Widget::TextSelection{
		.action_callback = [](const std::string &selection) -> ui::Action {
			auto group_type = m2g::pb::GroupType::NO_GROUP;
			m2g::pb::GroupType_Parse(selection, &group_type);
			std::get<ledit::State::PlaceMode>(LEVEL.level_editor_state->mode).select_group_type(group_type);
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant place_mode_right_hud_group_instance_selection = ui::Blueprint::Widget::IntegerSelection{
		.min_value = 0,
		.max_value = 999,
		.initial_value = 0,
		.action_callback = [](int selection) -> ui::Action {
			std::get<ledit::State::PlaceMode>(LEVEL.level_editor_state->mode).select_group_instance(selection);
			return ui::Action::CONTINUE;
		}
};
ui::Blueprint place_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				ui::Blueprint::Widget{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = place_mode_title
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = place_mode_right_hud_object_type_selection
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 11, .w = 11, .h = 4,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = place_mode_right_hud_group_type_selection
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 16, .w = 11, .h = 4,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = place_mode_right_hud_group_instance_selection
				}
		}
};

const ui::Blueprint::Widget::Variant remove_mode_title = ui::Blueprint::Widget::Text{
		.initial_text = "REMOVE"
};
const ui::Blueprint remove_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				ui::Blueprint::Widget{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = remove_mode_title
				}
		}
};

const ui::Blueprint::Widget::Variant pick_mode_title = ui::Blueprint::Widget::Text{
		.initial_text = "PICK"
};
const ui::Blueprint::Widget::Variant pick_mode_right_hud_ground_selection = ui::Blueprint::Widget::TextSelection{
		.list = {"Background", "Foreground"},
		.action_callback = [](const std::string& selection) -> ui::Action {
			if (selection == "Background") {
				std::get<ledit::State::PickMode>(LEVEL.level_editor_state->mode).pick_foreground = false;
			} else if (selection == "Foreground") {
				std::get<ledit::State::PickMode>(LEVEL.level_editor_state->mode).pick_foreground = true;
			}
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint pick_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				ui::Blueprint::Widget{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = pick_mode_title
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = pick_mode_right_hud_ground_selection
				}
		}
};

const ui::Blueprint::Widget::Variant select_mode_title = ui::Blueprint::Widget::Text{
		.initial_text = "SELECT"
};
const ui::Blueprint::Widget::Variant select_mode_right_hud_shift_right_button = ui::Blueprint::Widget::Text{
		.initial_text = "Shift Right",
		.action_callback = []() -> ui::Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).shift_right();
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant select_mode_right_hud_shift_down_button = ui::Blueprint::Widget::Text{
		.initial_text = "Shift Down",
		.action_callback = []() -> ui::Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).shift_down();
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant select_mode_right_hud_copy_button = ui::Blueprint::Widget::Text{
		.initial_text = "Copy",
		.action_callback = []() -> ui::Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).copy();
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant select_mode_right_hud_paste_bg_button = ui::Blueprint::Widget::Text{
		.initial_text = "Paste BG",
		.action_callback = []() -> ui::Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).paste_bg();
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant select_mode_right_hud_paste_fg_button = ui::Blueprint::Widget::Text{
		.initial_text = "Paste FG",
		.action_callback = []() -> ui::Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).paste_fg();
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant select_mode_right_hud_erase_button = ui::Blueprint::Widget::Text{
		.initial_text = "Erase",
		.action_callback = []() -> ui::Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).erase();
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant select_mode_right_hud_remove_button = ui::Blueprint::Widget::Text{
		.initial_text = "Remove",
		.action_callback = []() -> ui::Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).remove();
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint select_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				ui::Blueprint::Widget{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = select_mode_title
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 6, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_shift_right_button
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 10, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_shift_down_button
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 14, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_copy_button
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 18, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_paste_bg_button
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 22, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_paste_fg_button
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 26, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_erase_button
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 30, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_remove_button
				}
		}
};

const ui::Blueprint::Widget::Variant shift_mode_title = ui::Blueprint::Widget::Text{
		.initial_text = "SHIFT"
};
const ui::Blueprint::Widget::Variant shift_mode_right_hud_shift_direction_selection = ui::Blueprint::Widget::TextSelection{
		.list = {"Right", "Down", "Right & Down"},
		.action_callback = [](const std::string& selection) -> ui::Action {
			if (selection == "Right") {
				std::get<ledit::State::ShiftMode>(LEVEL.level_editor_state->mode).shift_type = ledit::State::ShiftMode::ShiftType::RIGHT;
			} else if (selection == "Down") {
				std::get<ledit::State::ShiftMode>(LEVEL.level_editor_state->mode).shift_type = ledit::State::ShiftMode::ShiftType::DOWN;
			} else if (selection == "Right & Down") {
				std::get<ledit::State::ShiftMode>(LEVEL.level_editor_state->mode).shift_type = ledit::State::ShiftMode::ShiftType::RIGHT_N_DOWN;
			}
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint shift_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {
				ui::Blueprint::Widget{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = shift_mode_title
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = shift_mode_right_hud_shift_direction_selection
				}
		}
};

const ui::Blueprint::Widget::Variant save_confirmation_text = ui::Blueprint::Widget::Text{
		.initial_text = "Are you sure?"
};
const ui::Blueprint::Widget::Variant save_confirmation_yes_button = ui::Blueprint::Widget::Text{
		.initial_text = "YES",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->save();
			return ui::Action::RETURN;
		},
		.kb_shortcut = SDL_SCANCODE_Y
};
const ui::Blueprint::Widget::Variant save_confirmation_no_button = ui::Blueprint::Widget::Text{
		.initial_text = "NO",
		.action_callback = []() -> ui::Action { return ui::Action::RETURN; },
		.kb_shortcut = SDL_SCANCODE_N
};
const ui::Blueprint save_confirmation = {
		.w = 7, .h = 5,
		.border_width_px = 2,
		.widgets = {
				ui::Blueprint::Widget{
						.x = 1, .y = 1, .w = 5, .h = 1,
						.variant = save_confirmation_text
				},
				ui::Blueprint::Widget{
						.x = 1, .y = 3, .w = 2, .h = 1,
						.variant = save_confirmation_yes_button
				},
				ui::Blueprint::Widget{
						.x = 4, .y = 3, .w = 2, .h = 1,
						.variant = save_confirmation_no_button
				}
		}
};

const ui::Blueprint::Widget::Variant left_hud_paint_button = ui::Blueprint::Widget::Text{
		.initial_text = std::string{m2::level_editor::paint_button_label},
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->activate_paint_mode();
			// Fill tile selector with editor-enabled sprites
			auto& list = std::get<ui::Blueprint::Widget::ImageSelection>(paint_mode_right_hud.widgets[1].variant).list;
			std::copy(std::begin(GAME.level_editor_background_sprites), std::end(GAME.level_editor_background_sprites), std::back_inserter(list));

			LEVEL.right_hud_ui_state = ui::State(&paint_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_P
};
const ui::Blueprint::Widget::Variant left_hud_erase_button = ui::Blueprint::Widget::Text{
		.initial_text = "ERASE",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->activate_erase_mode();
			LEVEL.right_hud_ui_state = ui::State(&erase_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_E
};
const ui::Blueprint::Widget::Variant left_hud_place_button = ui::Blueprint::Widget::Text{
		.initial_text = std::string{m2::level_editor::place_button_label},
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->activate_place_mode();
			// Fill object type selector with editor-enabled object types
			auto& object_type_selection = std::get<ui::Blueprint::Widget::TextSelection>(place_mode_right_hud.widgets[1].variant);
			if (object_type_selection.list.empty()) {
				for (auto& [obj_type, spt] : GAME.level_editor_object_sprites) {
					object_type_selection.list.emplace_back(m2g::pb::ObjectType_Name(obj_type));
				}
			}
			// Fill group type selector
			auto& group_type_selection = std::get<ui::Blueprint::Widget::TextSelection>(place_mode_right_hud.widgets[2].variant);
			if (group_type_selection.list.empty()) {
				for (int e  = 0; e < protobuf::enum_value_count<m2g::pb::GroupType>(); ++e) {
					group_type_selection.list.emplace_back(protobuf::enum_name<m2g::pb::GroupType>(e));
				}
			}

			LEVEL.right_hud_ui_state = ui::State(&place_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_O
};
const ui::Blueprint::Widget::Variant left_hud_remove_button = ui::Blueprint::Widget::Text{
		.initial_text = "REMOVE",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->activate_remove_mode();
			LEVEL.right_hud_ui_state = ui::State(&remove_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_R
};
const ui::Blueprint::Widget::Variant left_hud_pick_button = ui::Blueprint::Widget::Text{
		.initial_text = "PICK",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->activate_pick_mode();
			LEVEL.right_hud_ui_state = ui::State(&pick_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_R
};
const ui::Blueprint::Widget::Variant left_hud_select_button = ui::Blueprint::Widget::Text{
		.initial_text = "SELECT",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->activate_select_mode();
			LEVEL.right_hud_ui_state = ui::State(&select_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant left_hud_shift_button = ui::Blueprint::Widget::Text{
		.initial_text = "SHIFT",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->activate_shift_mode();
			LEVEL.right_hud_ui_state = ui::State(&shift_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant left_hud_cancel_button = ui::Blueprint::Widget::Text{
		.initial_text = "CANCEL",
		.action_callback = []() -> ui::Action {
			LEVEL.level_editor_state->deactivate_mode();
			LEVEL.right_hud_ui_state = ui::State(&level_editor::ui::right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_X
};
const ui::Blueprint::Widget::Variant left_hud_gridlines_button = ui::Blueprint::Widget::Text{
		.initial_text = "GRID",
		.action_callback = []() -> ui::Action {
			LEVEL.toggle_grid();
			return ui::Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_G
};
const ui::Blueprint::Widget::Variant left_hud_save_button = ui::Blueprint::Widget::Text{
		.initial_text = "SAVE",
		.action_callback = []() -> ui::Action {
			execute_blocking(&save_confirmation);
			return ui::Action::CONTINUE;
		}
};
const ui::Blueprint::Widget::Variant left_hud_coordinates = ui::Blueprint::Widget::Text{
		.initial_text = "0,0",
		.update_callback = []() {
			auto mouse_position = GAME.mouse_position_world_m().iround();
			return std::make_pair(ui::Action::CONTINUE, std::to_string(mouse_position.x) + ',' + std::to_string(mouse_position.y));
		}
};
const m2::ui::Blueprint m2::level_editor::ui::left_hud = {
		.w = 19, .h = 72,
		.border_width_px = 0,
		.background_color = {50, 50, 50, 255},
		.widgets = {
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_paint_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 6, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_erase_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 10, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_place_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 14, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_remove_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 18, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_pick_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 22, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_select_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 26, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_shift_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 30, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_cancel_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 62, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_gridlines_button
				},
				m2::ui::Blueprint::Widget{
						.x = 4, .y = 66, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_save_button
				},
				m2::ui::Blueprint::Widget{
						.x = 0, .y = 70, .w = 19, .h = 2,
						.border_width_px = 0,
						.variant = left_hud_coordinates
				}
		}
};

const ui::Blueprint m2::level_editor::ui::right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 0,
		.background_color = {50, 50, 50, 255}
};
