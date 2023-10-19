#include <m2/level_editor/Ui.h>
#include "m2/Game.h"
#include "m2/object/Ghost.h"
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/IntegerSelection.h>
#include <m2/ui/widget/ImageSelection.h>
#include <m2/ui/widget/Text.h>

using namespace m2;
using namespace m2::ui;

const widget::TextBlueprint paint_mode_title = {
	.initial_text = "PAINT"
};
const widget::IntegerSelectionBlueprint paint_mode_layer_selection = {
		.min_value = 0,
		.max_value = 3,
		.initial_value = 0,
		.on_action = [](const widget::IntegerSelection& self) -> Action {
			std::get<ledit::State::PaintMode>(LEVEL.level_editor_state->mode).selected_layer = static_cast<BackgroundLayer>(self.value());
			return Action::CONTINUE;
		}
};
const widget::ImageSelectionBlueprint paint_mode_image_selection = {
		.on_action = [](const widget::ImageSelection& self) -> Action {
			std::get<ledit::State::PaintMode>(LEVEL.level_editor_state->mode).select_sprite_type(self.selection());
			return Action::CONTINUE;
		}
};
Blueprint paint_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {50, 50, 50, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = paint_mode_title
				},
				WidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 6,
						.border_width_px = 1,
						.variant = paint_mode_layer_selection
				},
				WidgetBlueprint{
						.x = 4, .y = 13, .w = 11, .h = 14,
						.border_width_px = 1,
						.variant = paint_mode_image_selection
				}
		}
};

const widget::TextBlueprint erase_mode_title = {
		.initial_text = "ERASE"
};
const widget::IntegerSelectionBlueprint erase_mode_layer_selection = {
		.min_value = 0,
		.max_value = 3,
		.initial_value = 0,
		.on_action = [](const widget::IntegerSelection& self) -> Action {
			std::get<ledit::State::EraseMode>(LEVEL.level_editor_state->mode).selected_layer = static_cast<BackgroundLayer>(self.value());
			return Action::CONTINUE;
		}
};
const Blueprint erase_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {50, 50, 50, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = erase_mode_title
				},
				WidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 6,
						.border_width_px = 1,
						.variant = erase_mode_layer_selection
				},
		}
};

const widget::TextBlueprint place_mode_title = {
		.initial_text = "PLACE"
};
const widget::TextSelectionBlueprint place_mode_right_hud_object_type_selection = {
		.on_action = [](const widget::TextSelection& self) -> Action {
			auto object_type = m2g::pb::ObjectType::NO_OBJECT;
			m2g::pb::ObjectType_Parse(self.selection(), &object_type);
			std::get<ledit::State::PlaceMode>(LEVEL.level_editor_state->mode).select_object_type(object_type);
			return Action::CONTINUE;
		}
};
const widget::TextSelectionBlueprint place_mode_right_hud_group_type_selection = {
		.on_action = [](const widget::TextSelection& self) -> Action {
			auto group_type = m2g::pb::GroupType::NO_GROUP;
			m2g::pb::GroupType_Parse(self.selection(), &group_type);
			std::get<ledit::State::PlaceMode>(LEVEL.level_editor_state->mode).select_group_type(group_type);
			return Action::CONTINUE;
		}
};
const widget::IntegerSelectionBlueprint place_mode_right_hud_group_instance_selection = {
		.min_value = 0,
		.max_value = 999,
		.initial_value = 0,
		.on_action = [](const widget::IntegerSelection& self) -> Action {
			std::get<ledit::State::PlaceMode>(LEVEL.level_editor_state->mode).select_group_instance(self.value());
			return Action::CONTINUE;
		}
};
Blueprint place_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {50, 50, 50, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = place_mode_title
				},
				WidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = place_mode_right_hud_object_type_selection
				},
				WidgetBlueprint{
						.x = 4, .y = 11, .w = 11, .h = 4,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = place_mode_right_hud_group_type_selection
				},
				WidgetBlueprint{
						.x = 4, .y = 16, .w = 11, .h = 4,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = place_mode_right_hud_group_instance_selection
				}
		}
};

const widget::TextBlueprint remove_mode_title = {
		.initial_text = "REMOVE"
};
const Blueprint remove_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {50, 50, 50, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = remove_mode_title
				}
		}
};

const widget::TextBlueprint pick_mode_title = {
		.initial_text = "PICK"
};
const widget::TextSelectionBlueprint pick_mode_right_hud_ground_selection = {
		.initial_list = {"Background", "Foreground"},
		.on_action = [](const widget::TextSelection& self) -> Action {
			const auto& selection = self.selection();
			if (selection == "Background") {
				std::get<ledit::State::PickMode>(LEVEL.level_editor_state->mode).pick_foreground = false;
			} else if (selection == "Foreground") {
				std::get<ledit::State::PickMode>(LEVEL.level_editor_state->mode).pick_foreground = true;
			}
			return Action::CONTINUE;
		}
};
const widget::IntegerSelectionBlueprint pick_mode_layer_selection = {
		.min_value = 0,
		.max_value = 3,
		.initial_value = 0,
		.on_action = [](const widget::IntegerSelection& self) -> Action {
			std::get<ledit::State::PickMode>(LEVEL.level_editor_state->mode).selected_layer = static_cast<BackgroundLayer>(self.value());
			return Action::CONTINUE;
		}
};
const Blueprint pick_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {50, 50, 50, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = pick_mode_title
				},
				WidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = pick_mode_right_hud_ground_selection
				},
				WidgetBlueprint{
						.x = 4, .y = 11, .w = 11, .h = 6,
						.border_width_px = 1,
						.variant = pick_mode_layer_selection
				},
		}
};

const widget::TextBlueprint select_mode_title = {
		.initial_text = "SELECT"
};
const widget::IntegerSelectionBlueprint select_mode_layer_selection = {
		.min_value = 0,
		.max_value = 3,
		.initial_value = 0,
		.on_action = [](const widget::IntegerSelection& self) -> Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).selected_layer = static_cast<BackgroundLayer>(self.value());
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint select_mode_right_hud_shift_right_button = {
		.initial_text = "Shift Right",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).shift_right();
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint select_mode_right_hud_shift_down_button = {
		.initial_text = "Shift Down",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).shift_down();
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint select_mode_right_hud_copy_button = {
		.initial_text = "Copy",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).copy();
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint select_mode_right_hud_paste_bg_button = {
		.initial_text = "Paste BG",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).paste_bg();
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint select_mode_right_hud_paste_fg_button = {
		.initial_text = "Paste FG",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).paste_fg();
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint select_mode_right_hud_erase_button = {
		.initial_text = "Erase",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).erase();
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint select_mode_right_hud_remove_button = {
		.initial_text = "Remove",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode).remove();
			return Action::CONTINUE;
		}
};
const Blueprint select_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {50, 50, 50, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = select_mode_title
				},
				WidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 6,
						.border_width_px = 1,
						.variant = select_mode_layer_selection
				},
				WidgetBlueprint{
						.x = 4, .y = 13, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_shift_right_button
				},
				WidgetBlueprint{
						.x = 4, .y = 17, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_shift_down_button
				},
				WidgetBlueprint{
						.x = 4, .y = 21, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_copy_button
				},
				WidgetBlueprint{
						.x = 4, .y = 25, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_paste_bg_button
				},
				WidgetBlueprint{
						.x = 4, .y = 29, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_paste_fg_button
				},
				WidgetBlueprint{
						.x = 4, .y = 33, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_erase_button
				},
				WidgetBlueprint{
						.x = 4, .y = 37, .w = 11, .h = 3,
						.border_width_px = 1,
						.variant = select_mode_right_hud_remove_button
				}
		}
};

const widget::TextBlueprint shift_mode_title = {
		.initial_text = "SHIFT"
};
const widget::TextSelectionBlueprint shift_mode_right_hud_shift_direction_selection = {
		.initial_list = {"Right", "Down", "Right & Down"},
		.on_action = [](const widget::TextSelection& self) -> Action {
			const auto& selection = self.selection();
			if (selection == "Right") {
				std::get<ledit::State::ShiftMode>(LEVEL.level_editor_state->mode).shift_type = ledit::State::ShiftMode::ShiftType::RIGHT;
			} else if (selection == "Down") {
				std::get<ledit::State::ShiftMode>(LEVEL.level_editor_state->mode).shift_type = ledit::State::ShiftMode::ShiftType::DOWN;
			} else if (selection == "Right & Down") {
				std::get<ledit::State::ShiftMode>(LEVEL.level_editor_state->mode).shift_type = ledit::State::ShiftMode::ShiftType::RIGHT_N_DOWN;
			}
			return Action::CONTINUE;
		}
};
const Blueprint shift_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {50, 50, 50, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = shift_mode_title
				},
				WidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = shift_mode_right_hud_shift_direction_selection
				}
		}
};

const widget::TextBlueprint left_hud_paint_button = {
		.initial_text = std::string{level_editor::paint_button_label},
		.kb_shortcut = SDL_SCANCODE_P,
		.on_action = [](MAYBE const widget::Text& self) -> Action {
				LEVEL.level_editor_state->activate_paint_mode();
			// Fill tile selector with editor-enabled sprites
			auto& list = std::get<widget::ImageSelectionBlueprint>(paint_mode_right_hud.widgets[2].variant).list;
			std::copy(std::begin(GAME.level_editor_background_sprites), std::end(GAME.level_editor_background_sprites), std::back_inserter(list));

				LEVEL.right_hud_ui_state.emplace(&paint_mode_right_hud);
				LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_erase_button = {
		.initial_text = "ERASE",
		.kb_shortcut = SDL_SCANCODE_E,
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			LEVEL.level_editor_state->activate_erase_mode();
			LEVEL.right_hud_ui_state.emplace(&erase_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_place_button = {
		.initial_text = std::string{level_editor::place_button_label},
		.kb_shortcut = SDL_SCANCODE_O,
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			LEVEL.level_editor_state->activate_place_mode();
			// Fill object type selector with editor-enabled object types
			auto& object_type_selection = std::get<widget::TextSelectionBlueprint>(place_mode_right_hud.widgets[1].variant);
			if (object_type_selection.initial_list.empty()) {
				for (auto& [obj_type, spt] : GAME.level_editor_object_sprites) {
					object_type_selection.initial_list.emplace_back(m2g::pb::ObjectType_Name(obj_type));
				}
			}
			// Fill group type selector
			auto& group_type_selection = std::get<widget::TextSelectionBlueprint>(place_mode_right_hud.widgets[2].variant);
			if (group_type_selection.initial_list.empty()) {
				for (int e  = 0; e < protobuf::enum_value_count<m2g::pb::GroupType>(); ++e) {
					group_type_selection.initial_list.emplace_back(protobuf::enum_name<m2g::pb::GroupType>(e));
				}
			}

			LEVEL.right_hud_ui_state.emplace(&place_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_remove_button = {
		.initial_text = "REMOVE",
		.kb_shortcut = SDL_SCANCODE_R,
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			LEVEL.level_editor_state->activate_remove_mode();
			LEVEL.right_hud_ui_state.emplace(&remove_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_pick_button = {
		.initial_text = "PICK",
		.kb_shortcut = SDL_SCANCODE_R,
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			LEVEL.level_editor_state->activate_pick_mode();
			LEVEL.right_hud_ui_state.emplace(&pick_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_select_button = {
		.initial_text = "SELECT",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			LEVEL.level_editor_state->activate_select_mode();
			LEVEL.right_hud_ui_state.emplace(&select_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_shift_button = {
		.initial_text = "SHIFT",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			LEVEL.level_editor_state->activate_shift_mode();
			LEVEL.right_hud_ui_state.emplace(&shift_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_cancel_button = {
		.initial_text = "CANCEL",
		.kb_shortcut = SDL_SCANCODE_X,
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			LEVEL.level_editor_state->deactivate_mode();
			LEVEL.right_hud_ui_state.emplace(&level_editor::ui::right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_gridlines_button = {
		.initial_text = "GRID",
		.kb_shortcut = SDL_SCANCODE_G,
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			if (LEVEL.dynamic_grid_lines_loader) {
				LEVEL.dynamic_grid_lines_loader.reset();
			} else {
				LEVEL.dynamic_grid_lines_loader.emplace(SDL_Color{127, 127, 255, 127});
			}
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_save_button = {
		.initial_text = "SAVE",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			LEVEL.level_editor_state->save();
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_coordinates = {
		.initial_text = "0,0",
		.on_update = [](MAYBE const widget::Text& self) {
			auto mouse_position = GAME.mouse_position_world_m().iround();
			return std::make_pair(Action::CONTINUE, std::to_string(mouse_position.x) + ',' + std::to_string(mouse_position.y));
		}
};
const Blueprint level_editor::ui::left_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {50, 50, 50, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_paint_button
				},
				WidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_erase_button
				},
				WidgetBlueprint{
						.x = 4, .y = 10, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_place_button
				},
				WidgetBlueprint{
						.x = 4, .y = 14, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_remove_button
				},
				WidgetBlueprint{
						.x = 4, .y = 18, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_pick_button
				},
				WidgetBlueprint{
						.x = 4, .y = 22, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_select_button
				},
				WidgetBlueprint{
						.x = 4, .y = 26, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_shift_button
				},
				WidgetBlueprint{
						.x = 4, .y = 30, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_cancel_button
				},
				WidgetBlueprint{
						.x = 4, .y = 62, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_gridlines_button
				},
				WidgetBlueprint{
						.x = 4, .y = 66, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_save_button
				},
				WidgetBlueprint{
						.x = 0, .y = 70, .w = 19, .h = 2,
						.border_width_px = 0,
						.variant = left_hud_coordinates
				}
		}
};

const Blueprint level_editor::ui::right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {50, 50, 50, 255}
};
