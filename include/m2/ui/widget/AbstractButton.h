#pragma once
#include "../Widget.h"

namespace m2::ui::widget {
	struct AbstractButton : public Widget {
		SDL_Scancode kb_shortcut;
		bool depressed;

		explicit AbstractButton(const WidgetBlueprint* blueprint);
		Action handle_events(Events& events) final;
	};
}
