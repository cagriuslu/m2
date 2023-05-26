#pragma once
#include "../Widget.h"

namespace m2::ui::widget {
	struct ProgressBar : public Widget {
		float progress;

		explicit ProgressBar(const WidgetBlueprint* blueprint);
		Action update_content() override;
		void draw() override;
	};
}
