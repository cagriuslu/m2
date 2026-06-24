#pragma once
#include <m2/common/video/Color.h>
#include <functional>

namespace m2::widget {
	// Forward declaration
	class ProgressBar;

	struct ProgressBarBlueprint {
		float initial_progress{};
		RGBA bar_color{};

		std::function<void(ProgressBar& self)> onCreate{};
		std::function<void(ProgressBar& self)> onUpdate{};
	};
}
