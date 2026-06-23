#pragma once
#include "../ui/UiPanelBlueprint.h"
#include <array>

namespace m2 {
	namespace sheeteditor {
		const std::map<pb::Fixture::FixtureTypeCase, std::string> gFixtureTypeNames{
			{pb::Fixture::FixtureTypeCase::kRectangle, "Rectangle"},
			{pb::Fixture::FixtureTypeCase::kCircle, "Circle"},
			{pb::Fixture::FixtureTypeCase::kChain, "Chain"},
		};

		constexpr RGBA SELECTION_COLOR = {0, 127, 255, 180};
		constexpr RGBA CONFIRMED_SELECTION_COLOR = {0, 255, 0, 80};
		constexpr RGBA CROSS_COLOR = {0, 127, 255, 255};
		constexpr RGBA CONFIRMED_CROSS_COLOR = {0, 255, 0, 127};
		constexpr RGBA CONFIRMED_CROSS_COLOR2 = {255, 0, 0, 127};
	}

	extern const UiPanelBlueprint sheet_editor_left_hud;
	extern const UiPanelBlueprint sheet_editor_right_hud;
	extern const UiPanelBlueprint sheet_editor_main_menu;
}
