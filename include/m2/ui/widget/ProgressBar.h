#pragma once
#include "../UiWidget.h"

namespace m2::widget {
	class ProgressBar : public UiWidget {
		float _progress;

	public:
		explicit ProgressBar(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		UiAction UpdateContent() override;
		void Draw() override;

	private:
		[[nodiscard]] const ProgressBarBlueprint& VariantBlueprint() const { return std::get<ProgressBarBlueprint>(blueprint->variant); }
	};
}
