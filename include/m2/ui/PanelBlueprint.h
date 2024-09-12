#pragma once
#include "WidgetBlueprint.h"
#include <SDL.h>
#include <vector>

namespace m2::ui {
	struct PanelBlueprint {
		int w{1}, h{1}; // unitless

		// Unitless border width. If non-zero, the drawn border is at least 1 pixel. Border is drawn *inside* the Rect
		// of the UI, thus it behaves like a padding at the same time.
		float border_width{0.001f};

		SDL_Color background_color{};
		bool cancellable{}; // TODO if there are multiple UI states, it's hard to tell which one needs to be cancelled. Handle cancellations with a hidden button
		std::vector<WidgetBlueprint> widgets;
	};
}
