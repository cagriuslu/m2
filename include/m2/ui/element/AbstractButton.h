#ifndef M2_ABSTRACTBUTTON_H
#define M2_ABSTRACTBUTTON_H

#include "../ElementState.h"

namespace m2::ui::element {
	struct AbstractButtonState : public ElementState {
		bool depressed;

		explicit AbstractButtonState(const ElementBlueprint* blueprint);

		SDL_Scancode get_keyboard_shortcut() const override;
		void set_depressed(bool state) override;
		Action action() override;
	};
}

#endif //M2_ABSTRACTBUTTON_H
