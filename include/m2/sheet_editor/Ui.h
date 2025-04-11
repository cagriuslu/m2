#pragma once
#include "../ui/UiPanelBlueprint.h"
#include <array>

namespace m2 {
	namespace sheet_editor {
		enum FixtureType {
			RECTANGLE = 0,
			CIRCLE = 1,
			CHAIN = 2
		};
		constexpr std::array<std::string, 3> gFixtureTypeName = {
			"Rectangle", // RECTANGLE
			"Circle", // CIRCLE
			"Chain" // CHAIN
		};
	}

	extern const UiPanelBlueprint sheet_editor_left_hud;
	extern const UiPanelBlueprint sheet_editor_right_hud;
	extern const UiPanelBlueprint sheet_editor_main_menu;
}
