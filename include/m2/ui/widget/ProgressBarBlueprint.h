#pragma once
#include <SDL.h>
#include <functional>

namespace m2::widget {
	// Forward declaration
	class ProgressBar;

	struct ProgressBarBlueprint {
		float initial_progress{};
		SDL_Color bar_color{};

		std::function<void(ProgressBar& self)> onCreate{};
		std::function<float(ProgressBar& self)> onUpdate{};
	};
}
