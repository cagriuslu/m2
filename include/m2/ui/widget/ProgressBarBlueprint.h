#pragma once
#include <m2/common/video/Color.h>
#include <m2g_SpriteType.pb.h>
#include <functional>
#include <variant>

namespace m2::widget {
	// Forward declaration
	class ProgressBar;

	// Either nothing, a sprite to display, or a callback that draws the marker visual
	using ProgressBarMarkerVisual = std::variant<std::monostate, m2g::pb::SpriteType, std::function<void(const ProgressBar&)>>;

	struct ProgressBarBlueprint {
		float initial_progress{};
		RGBA bar_color{};
		// Visual drawn at the end of the filled portion of the bar.
		ProgressBarMarkerVisual markerVisual{};

		std::function<void(ProgressBar& self)> onCreate{};
		std::function<void(ProgressBar& self)> onUpdate{};
	};
}
