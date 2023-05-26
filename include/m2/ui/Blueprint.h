#pragma once
#include "WidgetBlueprint.h"
#include <SDL.h>
#include <vector>

namespace m2::ui {
	struct Blueprint {
		unsigned w{1}, h{1}; // unitless
		unsigned border_width_px{1};
		SDL_Color background_color{};
		std::vector<WidgetBlueprint> widgets;
	};
}
