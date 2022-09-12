#include <m2/ui/Editor.h>
#include <m2/Game.hh>
#include <m2/object/Ghost.h>
#include <m2/object/Camera.h>
#include <m2g/SpriteBlueprint.h>

using namespace m2;
using namespace m2::ui;

const WidgetBlueprint::WidgetBlueprintVariant editor_paint_mode_right_hud_selected_sprite = wdg::ImageBlueprint{
	.update_callback = []() -> std::pair<Action,std::optional<SpriteKey>> {
		if (GAME.level->editor_paint_mode_selected_sprite < 0) {
			return {Action::CONTINUE, {}};
		} else {
			return {Action::CONTINUE, GAME.editor_bg_sprites[GAME.level->editor_paint_mode_selected_sprite]};
		}
	}
};
const WidgetBlueprint::WidgetBlueprintVariant editor_paint_mode_right_hud_left_arrow = wdg::TextBlueprint{
	.initial_text = "<",
	.action_callback = []() {
		GAME.level->editor_paint_mode_select_sprite(GAME.level->editor_paint_mode_selected_sprite - 1);
		return Action::CONTINUE;
	}
};
const WidgetBlueprint::WidgetBlueprintVariant editor_paint_mode_right_hud_right_arrow = wdg::TextBlueprint{
	.initial_text = ">",
	.action_callback = []() {
		GAME.level->editor_paint_mode_select_sprite(GAME.level->editor_paint_mode_selected_sprite + 1);
		return Action::CONTINUE;
	}
};
const UIBlueprint editor_paint_mode_right_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1,
	.widgets = {
		WidgetBlueprint{
			.x = 4, .y = 4, .w = 11, .h = 11,
			.border_width_px = 1,
			.variant = editor_paint_mode_right_hud_selected_sprite
		},
		WidgetBlueprint{
			.x = 4, .y = 16, .w = 5, .h = 5,
			.border_width_px = 1,
			.variant = editor_paint_mode_right_hud_left_arrow
		},
		WidgetBlueprint{
			.x = 10, .y = 16, .w = 5, .h = 5,
			.border_width_px = 1,
			.variant = editor_paint_mode_right_hud_right_arrow
		}
	}
};

const WidgetBlueprint::WidgetBlueprintVariant editor_left_hud_paint_button = wdg::TextBlueprint{
	.initial_text = "Paint",
	.action_callback = []() -> Action {
		GAME.level->activate_mode(Level::EditorMode::PAINT);
		GAME.rightHudUIState = UIState(&editor_paint_mode_right_hud);
		GAME.rightHudUIState->update_positions(GAME.rightHudRect);
		return Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_P
};
const WidgetBlueprint::WidgetBlueprintVariant editor_left_hud_erase_button = wdg::TextBlueprint{
	.initial_text = "Erase",
	.action_callback = []() -> Action {
		// TODO
		return Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_E
};
const WidgetBlueprint::WidgetBlueprintVariant editor_left_hud_place_button = wdg::TextBlueprint{
	.initial_text = "Place",
	.action_callback = []() -> Action {
		// TODO
		return Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_O
};
const WidgetBlueprint::WidgetBlueprintVariant editor_left_hud_remove_button = wdg::TextBlueprint{
	.initial_text = "Remove",
	.action_callback = []() -> Action {
		// TODO
		return Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_R
};
const WidgetBlueprint::WidgetBlueprintVariant editor_left_hud_cancel_button = wdg::TextBlueprint{
	.initial_text = "Cancel",
	.action_callback = []() -> Action {
		GAME.level->activate_mode(Level::EditorMode::NONE);
		GAME.rightHudUIState = UIState(&editor_right_hud);
		GAME.rightHudUIState->update_positions(GAME.rightHudRect);
		return Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_C
};
const WidgetBlueprint::WidgetBlueprintVariant editor_left_hud_gridlines_button = wdg::TextBlueprint{
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
const WidgetBlueprint::WidgetBlueprintVariant editor_left_hud_save_button = wdg::TextBlueprint{
		.initial_text = "Save",
		.action_callback = []() -> Action {
			// TODO
			return Action::CONTINUE;
		},
		.kb_shortcut = SDL_SCANCODE_S
};
const m2::ui::UIBlueprint m2::ui::editor_left_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1,
	.widgets = {
		WidgetBlueprint{
			.x = 4, .y = 4, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_paint_button
		},
		WidgetBlueprint{
			.x = 4, .y = 8, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_erase_button
		},
		WidgetBlueprint{
			.x = 4, .y = 12, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_place_button
		},
		WidgetBlueprint{
			.x = 4, .y = 16, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_remove_button
		},
		WidgetBlueprint{
			.x = 4, .y = 20, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 1,
			.variant = editor_left_hud_cancel_button
		},
		WidgetBlueprint{
			.x = 4, .y = 60, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_gridlines_button
		},
		WidgetBlueprint{
			.x = 4, .y = 64, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_save_button
		},
	}
};

const UIBlueprint m2::ui::editor_right_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1
};
