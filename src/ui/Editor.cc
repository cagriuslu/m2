#include <m2/ui/Editor.h>
#include <m2/Game.hh>
#include <m2g/SpriteBlueprint.h>

using namespace m2;
using namespace m2::ui;

const WidgetBlueprint::WidgetBlueprintVariant editor_right_hud_draw_selected_sprite = wdg::ImageBlueprint{
	.update_callback = []() -> std::pair<Action,std::optional<SpriteIndex>> {
		return {Action::CONTINUE, GAME.level->draw_sprite_index};
	}
};
const WidgetBlueprint::WidgetBlueprintVariant editor_right_hud_draw_left_arrow = wdg::TextBlueprint{
	.initial_text = "<",
	.action_callback = []() {
		if (GAME.level->draw_sprite_index) {
			GAME.level->draw_sprite_index--;
		}
		return Action::CONTINUE;
	}
};
const WidgetBlueprint::WidgetBlueprintVariant editor_right_hud_draw_right_arrow = wdg::TextBlueprint{
	.initial_text = ">",
	.action_callback = []() {
		if (GAME.level->draw_sprite_index < m2g::IMPL_EDITOR_SPRITE_N) {
			GAME.level->draw_sprite_index++;
		}
		return Action::CONTINUE;
	}
};
const UIBlueprint editor_right_hud_draw = {
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

const WidgetBlueprint::WidgetBlueprintVariant editor_left_hud_draw_button = wdg::TextBlueprint{
	.initial_text = "Draw",
	.action_callback = []() -> Action {
		GAME.level->editor_mode = Level::EditorMode::DRAW;
		GAME.rightHudUIState = UIState(&editor_right_hud_draw);
		GAME.rightHudUIState->update_positions(GAME.rightHudRect);
		return Action::CONTINUE;
	}
};
const WidgetBlueprint::WidgetBlueprintVariant editor_left_hud_cancel_button = wdg::TextBlueprint{
	.initial_text = "Cancel",
	.action_callback = []() -> Action {
		GAME.level->editor_mode = Level::EditorMode::NONE;
		GAME.rightHudUIState = UIState(&editor_right_hud);
		GAME.rightHudUIState->update_positions(GAME.rightHudRect);
		return Action::CONTINUE;
	}
};
const m2::ui::UIBlueprint m2::ui::editor_left_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1,
	.widgets = {
		WidgetBlueprint{
			.x = 4, .y = 4, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_draw_button
		},
		WidgetBlueprint{
			.x = 4, .y = 8, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 1,
			.variant = editor_left_hud_cancel_button
		}
	}
};

const UIBlueprint m2::ui::editor_right_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1
};
