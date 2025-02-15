#pragma once
#include "../ui/UiPanelBlueprint.h"

namespace m2 {
	constexpr int FIXTURE_LAYER_SELECTION_BACKGROUND_OPTION = 0;
	constexpr int FIXTURE_LAYER_SELECTION_FOREGROUND_OPTION = 1;

	constexpr int FIXTURE_SHAPE_SELECTION_RECTANGLE = 0;
	constexpr int FIXTURE_SHAPE_SELECTION_CIRCLE = 1;
	constexpr int FIXTURE_SHAPE_SELECTION_CHAIN_POINT = 2;

	extern const UiPanelBlueprint sheet_editor_left_hud;
	extern const UiPanelBlueprint sheet_editor_right_hud;
	extern const UiPanelBlueprint sheet_editor_main_menu;
}
