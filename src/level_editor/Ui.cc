#include <m2/level_editor/Ui.h>
#include <m2/ui/widget/ImageSelection.h>
#include <m2/ui/widget/IntegerInput.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/protobuf/Detail.h>
#include "m2/Game.h"
#include "m2/game/object/Ghost.h"

using namespace m2;

// Common widgets
const widget::IntegerInputBlueprint layer_selection = {
    .min_value = 0,
    .max_value = 3,
    .initial_value = 0,
    .on_create = [](MAYBE const widget::IntegerInput& self) -> std::optional<int> {
	    return I(std::get<ledit::State>(M2_LEVEL.type_state).selected_layer);
    },
    .on_action = [](const widget::IntegerInput& self) -> UiAction {
	    std::get<ledit::State>(M2_LEVEL.type_state).selected_layer = static_cast<BackgroundLayer>(self.value());
	    return MakeContinueAction();
    }};

const widget::TextBlueprint paint_mode_title = {.text = "PAINT"};
const widget::ImageSelectionBlueprint paint_mode_image_selection = {
    .on_action = [](const widget::ImageSelection& self) -> UiAction {
	    std::get<ledit::State::PaintMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode)
	        .select_sprite_type(self.selection());
	    return MakeContinueAction();
    }};
UiPanelBlueprint paint_mode_right_hud = {
	.name = "PaintModeRightHud",
    .w = 19,
    .h = 72,
    .background_color = {50, 50, 50, 255},
    .widgets = {
        UiWidgetBlueprint{.x = 4, .y = 2, .w = 11, .h = 3, .border_width = 0.0f, .variant = paint_mode_title},
        UiWidgetBlueprint{.x = 4, .y = 6, .w = 11, .h = 6, .variant = layer_selection},
        UiWidgetBlueprint{
            .x = 4, .y = 13, .w = 11, .h = 14, .variant = paint_mode_image_selection}}};

const widget::TextBlueprint erase_mode_title = {.text = "ERASE"};
const UiPanelBlueprint erase_mode_right_hud = {
	.name = "EraseModeRightHud",
    .w = 19,
    .h = 72,
    .background_color = {50, 50, 50, 255},
    .widgets = {
        UiWidgetBlueprint{.x = 4, .y = 2, .w = 11, .h = 3, .border_width = 0.0f, .variant = erase_mode_title},
        UiWidgetBlueprint{.x = 4, .y = 6, .w = 11, .h = 6, .variant = layer_selection},
    }};

const widget::TextBlueprint place_mode_title = {.text = "PLACE"};
const widget::TextSelectionBlueprint place_mode_right_hud_object_type_selection = {
    .on_action = [](widget::TextSelection& self) -> UiAction {
	    auto object_type = static_cast<m2g::pb::ObjectType>(std::get<int>(self.selections()[0]));
	    std::get<ledit::State::PlaceMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).select_object_type(object_type);
	    return MakeContinueAction();
    }};
const widget::TextSelectionBlueprint place_mode_right_hud_group_type_selection = {
    .on_action = [](widget::TextSelection& self) -> UiAction {
	    auto group_type = static_cast<m2g::pb::GroupType>(std::get<int>(self.selections()[0]));
	    std::get<ledit::State::PlaceMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).select_group_type(group_type);
	    return MakeContinueAction();
    }};
const widget::IntegerInputBlueprint place_mode_right_hud_group_instance_selection = {
    .min_value = 0, .max_value = 999, .initial_value = 0, .on_action = [](const widget::IntegerInput& self) -> UiAction {
	    std::get<ledit::State::PlaceMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode)
	        .select_group_instance(self.value());
	    return MakeContinueAction();
    }};
UiPanelBlueprint place_mode_right_hud = {
	.name = "PlaceModeRightHud",
    .w = 19,
    .h = 72,
    .background_color = {50, 50, 50, 255},
    .widgets = {
        UiWidgetBlueprint{.x = 4, .y = 2, .w = 11, .h = 3, .border_width = 0.0f, .variant = place_mode_title},
        UiWidgetBlueprint{
            .x = 4,
            .y = 6,
            .w = 11,
            .h = 4,
            .variant = place_mode_right_hud_object_type_selection},
        UiWidgetBlueprint{
            .x = 4,
            .y = 11,
            .w = 11,
            .h = 4,
            .variant = place_mode_right_hud_group_type_selection},
        UiWidgetBlueprint{
            .x = 4,
            .y = 16,
            .w = 11,
            .h = 4,
            .variant = place_mode_right_hud_group_instance_selection}}};

const widget::TextBlueprint remove_mode_title = {.text = "REMOVE"};
const UiPanelBlueprint remove_mode_right_hud = {
	.name = "RemoveModeRightHud",
    .w = 19,
    .h = 72,
    .background_color = {50, 50, 50, 255},
    .widgets = {UiWidgetBlueprint{.x = 4, .y = 2, .w = 11, .h = 3, .border_width = 0.0f, .variant = remove_mode_title}}};

const widget::TextBlueprint pick_mode_title = {.text = "PICK"};
const widget::TextSelectionBlueprint pick_mode_right_hud_ground_selection = {
    .options = {{"Background", "Background"}, {"Foreground", "Foreground"}},
	.on_action = [](const widget::TextSelection& self) -> UiAction {
	    if (auto selection = std::get<std::string>(self.selections()[0]); selection == "Background") {
		    std::get<ledit::State::PickMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).pick_foreground = false;
	    } else if (selection == "Foreground") {
		    std::get<ledit::State::PickMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).pick_foreground = true;
	    }
	    return MakeContinueAction();
    }};
const UiPanelBlueprint pick_mode_right_hud = {
	.name = "PickModeRightHud",
    .w = 19,
    .h = 72,
    .background_color = {50, 50, 50, 255},
    .widgets = {
        UiWidgetBlueprint{.x = 4, .y = 2, .w = 11, .h = 3, .border_width = 0, .variant = pick_mode_title},
        UiWidgetBlueprint{
            .x = 4, .y = 6, .w = 11, .h = 4, .variant = pick_mode_right_hud_ground_selection},
        UiWidgetBlueprint{.x = 4, .y = 11, .w = 11, .h = 6, .variant = layer_selection},
    }};

const widget::TextBlueprint select_mode_title = {.text = "SELECT"};
const widget::TextBlueprint select_mode_right_hud_shift_right_button = {
    .text = "Shift Right", .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State::SelectMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).shift_right();
	    return MakeContinueAction();
    }};
const widget::TextBlueprint select_mode_right_hud_shift_down_button = {
    .text = "Shift Down", .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State::SelectMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).shift_down();
	    return MakeContinueAction();
    }};
const widget::TextBlueprint select_mode_right_hud_copy_button = {
    .text = "Copy", .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State::SelectMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).copy();
	    return MakeContinueAction();
    }};
const widget::TextBlueprint select_mode_right_hud_paste_bg_button = {
    .text = "Paste BG", .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State::SelectMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).paste_bg();
	    return MakeContinueAction();
    }};
const widget::TextBlueprint select_mode_right_hud_paste_fg_button = {
    .text = "Paste FG", .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State::SelectMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).paste_fg();
	    return MakeContinueAction();
    }};
const widget::TextBlueprint select_mode_right_hud_erase_button = {
    .text = "Erase", .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State::SelectMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).erase();
	    return MakeContinueAction();
    }};
const widget::TextBlueprint select_mode_right_hud_remove_button = {
    .text = "Remove", .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State::SelectMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).remove();
	    return MakeContinueAction();
    }};
const UiPanelBlueprint m2::level_editor::fill_dialog = {
	.name = "FillDialog",
    .w = 160,
    .h = 90,
    .background_color = {0, 0, 0, 255},
    .widgets = {
        UiWidgetBlueprint{
            .x = 5, .y = 5, .w = 150, .h = 70,
            .variant = widget::TextSelectionBlueprint{
                .line_count = 10,
                .allow_multiple_selection = true,
                .on_create = [](MAYBE widget::TextSelection &self) {
					widget::TextSelectionBlueprint::Options options;
	                for (auto sprite : M2_GAME.level_editor_background_sprites) {
						options.emplace_back(widget::TextSelectionBlueprint::Option{pb::enum_name(sprite), I(sprite)});
	                }
					self.set_options(std::move(options));
                }
            }
        },
        UiWidgetBlueprint{
            .x = 60, .y = 80, .w = 40, .h = 5,
            .variant = widget::TextBlueprint {
                .text = "Fill",
                .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	                auto text_list_selection = self.parent().find_first_widget_of_type<widget::TextSelection>();

	                std::vector<m2g::pb::SpriteType> sprite_types;
	                for (const auto& selection : text_list_selection->selections()) {
						sprite_types.emplace_back(static_cast<m2g::pb::SpriteType>(std::get<int>(selection)));
	                }
	                std::get<ledit::State::SelectMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).rfill_sprite_types = sprite_types;

	                return MakeReturnAction(); // TODO Return value
                }
            }
        }
    }
};
const widget::TextBlueprint select_mode_right_hud_fill_button = {
    .text = "RFill",
    .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    return std::get<ledit::State::SelectMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).rfill();
    }
};
const UiPanelBlueprint select_mode_right_hud = {
	.name = "SelectModeRightHud",
    .w = 19,
    .h = 72,
    .background_color = {50, 50, 50, 255},
    .widgets = {
        UiWidgetBlueprint{.x = 4, .y = 2, .w = 11, .h = 3, .border_width = 0, .variant = select_mode_title},
        UiWidgetBlueprint{.x = 4, .y = 6, .w = 11, .h = 6, .variant = layer_selection},
        UiWidgetBlueprint{
            .x = 4,
            .y = 13,
            .w = 11,
            .h = 3,
            .variant = select_mode_right_hud_shift_right_button},
        UiWidgetBlueprint{
            .x = 4, .y = 17, .w = 11, .h = 3, .variant = select_mode_right_hud_shift_down_button},
        UiWidgetBlueprint{
            .x = 4, .y = 21, .w = 11, .h = 3, .variant = select_mode_right_hud_copy_button},
        UiWidgetBlueprint{
            .x = 4, .y = 25, .w = 11, .h = 3, .variant = select_mode_right_hud_paste_bg_button},
        UiWidgetBlueprint{
            .x = 4, .y = 29, .w = 11, .h = 3, .variant = select_mode_right_hud_paste_fg_button},
        UiWidgetBlueprint{
            .x = 4, .y = 33, .w = 11, .h = 3, .variant = select_mode_right_hud_erase_button},
        UiWidgetBlueprint{
            .x = 4, .y = 37, .w = 11, .h = 3, .variant = select_mode_right_hud_remove_button},
        UiWidgetBlueprint{
            .x = 4, .y = 41, .w = 11, .h = 3, .variant = select_mode_right_hud_fill_button}}};

const widget::TextBlueprint shift_mode_title = {.text = "SHIFT"};
const widget::TextSelectionBlueprint shift_mode_right_hud_shift_direction_selection = {
    .options = {{"Right", "Right"}, {"Down", "Down"}, {"Right & Down", "Right & Down"}}, .on_action = [](const widget::TextSelection& self) -> UiAction {
	    if (auto selection = std::get<std::string>(self.selections()[0]); selection == "Right") {
		    std::get<ledit::State::ShiftMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).shift_type =
		        ledit::State::ShiftMode::ShiftType::RIGHT;
	    } else if (selection == "Down") {
		    std::get<ledit::State::ShiftMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).shift_type =
		        ledit::State::ShiftMode::ShiftType::DOWN;
	    } else if (selection == "Right & Down") {
		    std::get<ledit::State::ShiftMode>(std::get<ledit::State>(M2_LEVEL.type_state).mode).shift_type =
		        ledit::State::ShiftMode::ShiftType::RIGHT_N_DOWN;
	    }
	    return MakeContinueAction();
    }};
const UiPanelBlueprint shift_mode_right_hud = {
	.name = "ShiftModeRightHud",
    .w = 19,
    .h = 72,
    .background_color = {50, 50, 50, 255},
    .widgets = {
        UiWidgetBlueprint{.x = 4, .y = 2, .w = 11, .h = 3, .border_width = 0, .variant = shift_mode_title},
        UiWidgetBlueprint{
            .x = 4,
            .y = 6,
            .w = 11,
            .h = 4,
            .variant = shift_mode_right_hud_shift_direction_selection}}};

const widget::TextBlueprint left_hud_paint_button = {
    .text = std::string{level_editor::paint_button_label},
    .kb_shortcut = SDL_SCANCODE_P,
    .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State>(M2_LEVEL.type_state).activate_paint_mode();
	    // Fill tile selector with editor-enabled sprites
	    auto& list = std::get<widget::ImageSelectionBlueprint>(paint_mode_right_hud.widgets[2].variant).list;
	    std::copy(
	        std::begin(M2_GAME.level_editor_background_sprites), std::end(M2_GAME.level_editor_background_sprites),
	        std::back_inserter(list));

	    M2_LEVEL.ReplaceRightHud(&paint_mode_right_hud, M2_GAME.Dimensions().RightHud());
	    return MakeContinueAction();
    }};
const widget::TextBlueprint left_hud_erase_button = {
    .text = "ERASE", .kb_shortcut = SDL_SCANCODE_E, .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State>(M2_LEVEL.type_state).activate_erase_mode();
	    M2_LEVEL.ReplaceRightHud(&erase_mode_right_hud, M2_GAME.Dimensions().RightHud());
	    return MakeContinueAction();
    }};
const widget::TextBlueprint left_hud_place_button = {
    .text = std::string{level_editor::place_button_label},
    .kb_shortcut = SDL_SCANCODE_O,
    .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State>(M2_LEVEL.type_state).activate_place_mode();
	    // Fill object type selector with editor-enabled object types
	    auto& object_type_selection = std::get<widget::TextSelectionBlueprint>(place_mode_right_hud.widgets[1].variant);
	    if (object_type_selection.options.empty()) {
		    for (auto& [obj_type, spt] : M2_GAME.object_main_sprites) {
			    object_type_selection.options.emplace_back(
					widget::TextSelectionBlueprint::Option{m2g::pb::ObjectType_Name(obj_type), I(obj_type)});
		    }
	    }
	    // Fill group type selector
	    auto& group_type_selection = std::get<widget::TextSelectionBlueprint>(place_mode_right_hud.widgets[2].variant);
	    if (group_type_selection.options.empty()) {
		    for (int e = 0; e < pb::enum_value_count<m2g::pb::GroupType>(); ++e) {
			    group_type_selection.options.emplace_back(
					widget::TextSelectionBlueprint::Option{
						pb::enum_name<m2g::pb::GroupType>(e), pb::enum_value<m2g::pb::GroupType>(e)});
		    }
	    }

	    M2_LEVEL.ReplaceRightHud(&place_mode_right_hud, M2_GAME.Dimensions().RightHud());
	    return MakeContinueAction();
    }};
const widget::TextBlueprint left_hud_remove_button = {
    .text = "REMOVE", .kb_shortcut = SDL_SCANCODE_R, .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State>(M2_LEVEL.type_state).activate_remove_mode();
	    M2_LEVEL.ReplaceRightHud(&remove_mode_right_hud, M2_GAME.Dimensions().RightHud());
	    return MakeContinueAction();
    }};
const widget::TextBlueprint left_hud_pick_button = {
    .text = "PICK", .kb_shortcut = SDL_SCANCODE_R, .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State>(M2_LEVEL.type_state).activate_pick_mode();
	    M2_LEVEL.ReplaceRightHud(&pick_mode_right_hud, M2_GAME.Dimensions().RightHud());
	    return MakeContinueAction();
    }};
const widget::TextBlueprint left_hud_select_button = {
    .text = "SELECT", .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State>(M2_LEVEL.type_state).activate_select_mode();
	    M2_LEVEL.ReplaceRightHud(&select_mode_right_hud, M2_GAME.Dimensions().RightHud());
	    return MakeContinueAction();
    }};
const widget::TextBlueprint left_hud_shift_button = {
    .text = "SHIFT", .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State>(M2_LEVEL.type_state).activate_shift_mode();
	    M2_LEVEL.ReplaceRightHud(&shift_mode_right_hud, M2_GAME.Dimensions().RightHud());
	    return MakeContinueAction();
    }};
const widget::TextBlueprint left_hud_cancel_button = {
    .text = "CANCEL", .kb_shortcut = SDL_SCANCODE_X, .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<ledit::State>(M2_LEVEL.type_state).deactivate_mode();
	    M2_LEVEL.ReplaceRightHud(&level_editor::RightHudBlueprint, M2_GAME.Dimensions().RightHud());
	    return MakeContinueAction();
    }};
const widget::TextBlueprint left_hud_gridlines_button = {
    .text = "GRID", .kb_shortcut = SDL_SCANCODE_G, .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    if (M2_LEVEL.dynamic_grid_lines_loader) {
		    M2_LEVEL.dynamic_grid_lines_loader.reset();
	    } else {
		    M2_LEVEL.dynamic_grid_lines_loader.emplace(SDL_Color{127, 127, 255, 127});
	    }
	    return MakeContinueAction();
    }};
const widget::TextBlueprint left_hud_coordinates = {
    .text = "0,0", .on_update = [](MAYBE widget::Text& self) {
	    const auto mouse_position = M2_GAME.MousePositionWorldM().iround();
		self.set_text(m2::ToString(mouse_position.x) + ',' + m2::ToString(mouse_position.y));
	    return MakeContinueAction();
    }};
const UiPanelBlueprint level_editor::LeftHudBlueprint = {
	.name = "LeftHud",
    .w = 19,
    .h = 72,
    .background_color = {50, 50, 50, 255},
    .widgets = {
        UiWidgetBlueprint{
            .x = 4,
            .y = 2,
            .w = 11,
            .h = 3,
            .variant = left_hud_paint_button},
        UiWidgetBlueprint{
            .x = 4,
            .y = 6,
            .w = 11,
            .h = 3,
            .variant = left_hud_erase_button},
        UiWidgetBlueprint{
            .x = 4,
            .y = 10,
            .w = 11,
            .h = 3,
            .variant = left_hud_place_button},
        UiWidgetBlueprint{
            .x = 4,
            .y = 14,
            .w = 11,
            .h = 3,
            .variant = left_hud_remove_button},
        UiWidgetBlueprint{
            .x = 4,
            .y = 18,
            .w = 11,
            .h = 3,
            .variant = left_hud_pick_button},
        UiWidgetBlueprint{
            .x = 4,
            .y = 22,
            .w = 11,
            .h = 3,
            .variant = left_hud_select_button},
        UiWidgetBlueprint{
            .x = 4,
            .y = 26,
            .w = 11,
            .h = 3,
            .variant = left_hud_shift_button},
        UiWidgetBlueprint{
            .x = 4,
            .y = 30,
            .w = 11,
            .h = 3,
            .variant = left_hud_cancel_button},
        UiWidgetBlueprint{
            .x = 4,
            .y = 66,
            .w = 11,
            .h = 3,
            .variant = left_hud_gridlines_button},
        UiWidgetBlueprint{.x = 0, .y = 70, .w = 19, .h = 2, .border_width = 0, .variant = left_hud_coordinates}}};

const UiPanelBlueprint level_editor::RightHudBlueprint = {
	.name = "RightHud",
    .w = 19, .h = 72, .background_color = {50, 50, 50, 255}, .widgets = {}};

const widget::TextBlueprint save_button = {
    .text = "Save", .kb_shortcut = SDL_SCANCODE_S, .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    if (auto success = ledit::State::save(); not success) {
    		M2_LEVEL.ShowMessage(success.error(), 8.0f);
    	}
	    return MakeContinueAction();
    }};
const widget::TextBlueprint quit_button = {
    .text = "Quit", .kb_shortcut = SDL_SCANCODE_Q, .on_action = [](MAYBE const widget::Text& self) -> UiAction {
	    return MakeQuitAction();
    }};
const UiPanelBlueprint level_editor::menu = {
	.name = "MainMenu",
    .w = 160,
    .h = 90,
    .border_width = 0,
    .background_color = {50, 50, 50, 255},
    .cancellable = true,
    .widgets = {
        UiWidgetBlueprint{
            .x = 70, .y = 40, .w = 20, .h = 10, .variant = save_button},
        UiWidgetBlueprint{
            .x = 70, .y = 70, .w = 20, .h = 10, .variant = quit_button}}};
