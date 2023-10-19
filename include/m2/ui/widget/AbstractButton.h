#pragma once
#include "../Widget.h"

namespace m2::ui::widget {
	struct AbstractButton : public Widget {
		SDL_Scancode kb_shortcut;
		bool depressed;

		explicit AbstractButton(State* parent, const WidgetBlueprint* blueprint);
		Action on_event(Events& events) final;

		// Helpers for external usage
		Action trigger_action();
	};
}
