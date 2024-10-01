#pragma once
#include "WidgetBlueprint.h"
#include "../Events.h"
#include <SDL.h>
#include <vector>

namespace m2::ui {
	// Forward declaration
	struct Panel;

	struct PanelBlueprint {
		int w{1}, h{1}; // unitless

		// Unitless border width. If non-zero, the drawn border is at least 1 pixel. Border is drawn *inside* the Rect
		// of the UI, thus it behaves like a padding at the same time.
		float border_width{0.001f};
		SDL_Color background_color{};

		bool cancellable{}; // TODO if there are multiple UI states, it's hard to tell which one needs to be cancelled. Handle cancellations with a hidden button
		bool ignore_events{false}; // If true, no events are delivered to the Panel
		float timeout_s{}; // If set, the Panel is destroyed once the timeout runs out

		std::function<void(Panel&)> on_create;
		std::function<Action(Panel& self, Events& events)> on_event;

		std::vector<WidgetBlueprint> widgets;
	};
}
