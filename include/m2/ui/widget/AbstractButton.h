#pragma once
#include "../UiWidget.h"

namespace m2::widget {
	struct AbstractButton : public UiWidget {
		SDL_Scancode kb_shortcut;
		bool depressed;

		explicit AbstractButton(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		UiAction on_event(Events& events) final;

		// Modifiers
		void recreate();
		UiAction trigger_action();
	};
}
