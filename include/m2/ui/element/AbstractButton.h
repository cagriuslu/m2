#ifndef M2_ABSTRACTBUTTON_H
#define M2_ABSTRACTBUTTON_H

#include "../ElementState.h"

namespace m2::ui::element {
	struct AbstractButtonState : public ElementState {
        SDL_Scancode kb_shortcut;
		bool depressed;

		explicit AbstractButtonState(const ElementBlueprint* blueprint);
        Action handle_events(Events& events) final;
	};
}

#endif //M2_ABSTRACTBUTTON_H
