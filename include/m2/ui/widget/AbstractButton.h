#pragma once
#include "../Widget.h"

namespace m2::ui::widget {
	struct AbstractButton : public Widget {
		SDL_Scancode kb_shortcut;
		bool depressed;

		explicit AbstractButton(Panel* parent, const WidgetBlueprint* blueprint);
		Action on_event(Events& events) final;

		// Modifiers
		void recreate();
		Action trigger_action();
	};
}
