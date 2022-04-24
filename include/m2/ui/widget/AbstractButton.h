#ifndef M2_ABSTRACTBUTTON_H
#define M2_ABSTRACTBUTTON_H

#include "../WidgetState.h"

namespace m2::ui::wdg {
	struct AbstractButtonState : public WidgetState {
        SDL_Scancode kb_shortcut;
		bool depressed;

		explicit AbstractButtonState(const WidgetBlueprint* blueprint);
        Action handle_events(Events& events) final;
	};
}

#endif //M2_ABSTRACTBUTTON_H
