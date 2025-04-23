#pragma once
#include "../ui/UiPanelBlueprint.h"
#include <array>

namespace m2 {
	namespace sheet_editor {
		const std::map<pb::Fixture::FixtureTypeCase, std::string> gFixtureTypeNames{
			{pb::Fixture::FixtureTypeCase::kRectangle, "Rectangle"},
			{pb::Fixture::FixtureTypeCase::kCircle, "Circle"},
			{pb::Fixture::FixtureTypeCase::kChain, "Chain"},
		};

		constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 180};
		constexpr SDL_Color CONFIRMED_SELECTION_COLOR = {0, 255, 0, 80};
		constexpr SDL_Color CROSS_COLOR = {0, 127, 255, 255};
		constexpr SDL_Color CONFIRMED_CROSS_COLOR = {0, 255, 0, 255};
	}

	extern const UiPanelBlueprint sheet_editor_left_hud;
	extern const UiPanelBlueprint sheet_editor_right_hud;
	extern const UiPanelBlueprint sheet_editor_main_menu;
}
