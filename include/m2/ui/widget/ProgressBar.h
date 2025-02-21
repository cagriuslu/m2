#pragma once
#include "../UiWidget.h"

namespace m2::widget {
	class ProgressBar : public UiWidget {
		float _progress;

	public:
		explicit ProgressBar(UiPanel* parent, const UiWidgetBlueprint* blueprint);

		UiAction UpdateContent() override;
		void Draw() override;

		// Accessors

		[[nodiscard]] float Progress() const { return _progress; }

		// Modifiers

		void SetProgress(const float p) { _progress = p; }

	private:
		[[nodiscard]] const ProgressBarBlueprint& VariantBlueprint() const { return std::get<ProgressBarBlueprint>(blueprint->variant); }
	};
}
