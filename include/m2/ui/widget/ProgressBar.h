#pragma once
#include "../UiWidget.h"

namespace m2::widget {
	class ProgressBar : public UiWidget {
		float _progress;

	public:
		explicit ProgressBar(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		UiAction on_update() override;
		void on_draw() override;

		// Modifiers
		void recreate();
	};
}
