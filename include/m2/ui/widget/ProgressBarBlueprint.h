#pragma once
#include <SDL.h>
#include <functional>

namespace m2::ui::widget {
	struct ProgressBarBlueprint {
		float initial_progress{};
		SDL_Color bar_color{};
		std::function<float(void)> update_callback;
	};
}
