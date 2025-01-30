#pragma once
#include "../UiWidget.h"

namespace m2::widget {
	struct AbstractButton : public UiWidget {
		SDL_Scancode kb_shortcut;
		bool depressed;

		explicit AbstractButton(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		UiAction HandleEvents(Events& events) final;

		// Modifiers

		UiAction trigger_action();
	};
}
