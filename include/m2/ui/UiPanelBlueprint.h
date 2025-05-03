#pragma once
#include "UiWidgetBlueprint.h"
#include "../Events.h"
#include <SDL.h>
#include <vector>

namespace m2 {
	// Forward declaration
	struct UiPanel;

	struct UiPanelBlueprint {
		/// Name of the panel used for searching and logging
		std::string name;

		/// Unitless width and height of the whole panel
		int w{1}, h{1};

		/// Unitless border width. If non-zero, the drawn border is at least 1 pixel. Border is drawn *inside* the Rect
		/// of the UI, thus it behaves like a padding at the same time.
		float border_width{0.001f};

		SDL_Color background_color{};

		bool cancellable{}; // TODO if there are multiple UI states, it's hard to tell which one needs to be cancelled. Handle cancellations with a hidden button

		/// If set to true, no events will be delivered to the UiPanel. This is usually used for semi-transparent
		/// message and notification boxes.
		bool ignore_events{false};

		float timeout_s{}; // If set, the UiPanel is destroyed once the timeout runs out

		std::function<void(UiPanel&)> onCreate{};
		std::function<UiAction(UiPanel&, Events&)> onEvent{};
		std::function<UiAction(UiPanel&)> onUpdate{};
		std::function<void()> onDestroy{};

		std::vector<UiWidgetBlueprint> widgets;
	};
}
