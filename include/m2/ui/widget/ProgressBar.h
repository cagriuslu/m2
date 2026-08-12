#pragma once
#include "../UiWidget.h"
#include <algorithm>

namespace m2::widget {
	class ProgressBar : public UiWidget {
		float _progress;
		ProgressBarMarkerVisual _markerVisual;

	public:
		explicit ProgressBar(UiPanel* parent, const UiWidgetBlueprint* blueprint);

		// Accessors

		[[nodiscard]] float Progress() const { return _progress; }
		[[nodiscard]] RectF MarkerVisualRect() const;

		// Modifiers

		void SetProgress(const float p) { _progress = std::clamp(p, 0.0f, 1.0f); }
		void SetMarkerVisual(ProgressBarMarkerVisual markerVisual);

	protected:
		UiAction OnUpdate() override;

		void OnDraw() override;

	private:
		[[nodiscard]] const ProgressBarBlueprint& VariantBlueprint() const { return std::get<ProgressBarBlueprint>(blueprint->variant); }
	};
}
