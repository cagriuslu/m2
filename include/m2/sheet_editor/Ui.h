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
	}

	extern const UiPanelBlueprint sheet_editor_left_hud;
	extern const UiPanelBlueprint sheet_editor_right_hud;
	extern const UiPanelBlueprint sheet_editor_main_menu;
}
