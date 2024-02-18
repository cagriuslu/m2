#pragma once
#include "WidgetBlueprint.h"
#include <SDL.h>
#include <vector>

namespace m2::ui {
	struct Blueprint {
		int w{1}, h{1}; // unitless
		unsigned border_width_px{1};
		SDL_Color background_color{};
		bool cancellable{}; // TODO if there are multiple UI states, it's hard to tell which one needs to be cancelled. Handle cancellations with a hidden button
		std::vector<WidgetBlueprint> widgets;
	};
}
