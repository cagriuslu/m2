#pragma once
#include "../UiWidget.h"

namespace m2::widget {
	class ProgressBar : public UiWidget {
		float _progress;

	public:
		explicit ProgressBar(UiPanel* parent, const UiWidgetBlueprint* blueprint);

		// Accessors

		[[nodiscard]] float Progress() const { return _progress; }

		// Modifiers

		void SetProgress(const float p) { _progress = p; }

	protected:
		UiAction OnUpdate() override;
		void OnDraw() override;

	private:
		[[nodiscard]] const ProgressBarBlueprint& VariantBlueprint() const { return std::get<ProgressBarBlueprint>(blueprint->variant); }
	};
}
