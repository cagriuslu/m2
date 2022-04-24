#ifndef M2_PROGRESSBAR_H
#define M2_PROGRESSBAR_H

#include "../WidgetState.h"
#include <functional>

namespace m2::ui::wdg {
	struct ProgressBarBlueprint {
		float initial_progress;
		SDL_Color bar_color;
		std::function<float(void)> update_callback;
	};

	struct ProgressBarState : public WidgetState {
		float progress;

		explicit ProgressBarState(const WidgetBlueprint* blueprint);
		Action update_content() override;
		void draw() override;
	};
}

#endif //M2_PROGRESSBAR_H
