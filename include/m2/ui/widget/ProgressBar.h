#pragma once
#include "../Widget.h"

namespace m2::ui::widget {
	class ProgressBar : public Widget {
		float _progress;

	public:
		explicit ProgressBar(Panel* parent, const WidgetBlueprint* blueprint);
		Action on_update() override;
		void on_draw() override;

		// Modifiers
		void recreate();
	};
}
