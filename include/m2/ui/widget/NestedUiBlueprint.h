#pragma once

namespace m2::ui {
	// Forward declaration
	struct PanelBlueprint;

	namespace widget {
		struct NestedUiBlueprint {
			const PanelBlueprint* ui{};
			int inner_w{}, inner_h{}; // Zero means nested UI should fit the available space
		};
	}
}
