#pragma once

namespace m2::ui {
	// Forward declaration
	struct Blueprint;

	namespace widget {
		struct NestedUiBlueprint {
			const Blueprint* ui{};
			int inner_w{}, inner_h{}; // Zero means nested UI should fit the available space
		};
	}
}
