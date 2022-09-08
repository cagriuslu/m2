#include <m2/ui/Editor.h>
#include <m2/Game.hh>
#include <m2/object/Ghost.h>
#include <m2/object/Camera.h>
#include <m2g/SpriteBlueprint.h>

using namespace m2;
using namespace m2::ui;

const WidgetBlueprint::WidgetBlueprintVariant editor_right_hud_draw_selected_sprite = wdg::ImageBlueprint{
	.update_callback = []() -> std::pair<Action,std::optional<SpriteKey>> {
		if (GAME.level->editor_paint_mode_selected_sprite < 0) {
			return {Action::CONTINUE, {}};
		} else {
			return {Action::CONTINUE, GAME.editor_sprites[GAME.level->editor_paint_mode_selected_sprite]};
		}
	}
};
const WidgetBlueprint::WidgetBlueprintVariant editor_right_hud_draw_left_arrow = wdg::TextBlueprint{
	.initial_text = "<",
	.action_callback = []() {
		GAME.level->editor_paint_mode_select_sprite(GAME.level->editor_paint_mode_selected_sprite - 1);
		return Action::CONTINUE;
	}
};
const WidgetBlueprint::WidgetBlueprintVariant editor_right_hud_draw_right_arrow = wdg::TextBlueprint{
	.initial_text = ">",
	.action_callback = []() {
		GAME.level->editor_paint_mode_select_sprite(GAME.level->editor_paint_mode_selected_sprite + 1);
		return Action::CONTINUE;
	}
};
const UIBlueprint editor_right_hud_paint = {
	.w = 19, .h = 72,
	.border_width_px = 1,
	.widgets = {
		WidgetBlueprint{
			.x = 4, .y = 4, .w = 11, .h = 11,
			.border_width_px = 1,
			.variant = editor_right_hud_draw_selected_sprite
		},
		WidgetBlueprint{
			.x = 4, .y = 16, .w = 5, .h = 5,
			.border_width_px = 1,
			.variant = editor_right_hud_draw_left_arrow
		},
		WidgetBlueprint{
			.x = 10, .y = 16, .w = 5, .h = 5,
			.border_width_px = 1,
			.variant = editor_right_hud_draw_right_arrow
		}
	}
};

const WidgetBlueprint::WidgetBlueprintVariant editor_left_hud_paint_button = wdg::TextBlueprint{
	.initial_text = "Paint",
	.action_callback = []() -> Action {
		GAME.level->activate_mode(Level::EditorMode::PAINT);
		GAME.rightHudUIState = UIState(&editor_right_hud_paint);
		GAME.rightHudUIState->update_positions(GAME.rightHudRect);
		return Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_P
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
			.padding_width_px = 1,
			.variant = editor_left_hud_cancel_button
		},
		WidgetBlueprint{
			.x = 4, .y = 64, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_gridlines_button
		},
	}
};

const UIBlueprint m2::ui::editor_right_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1
};
