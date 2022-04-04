#ifndef M2_BUTTONSTATICIMAGE_H
#define M2_BUTTONSTATICIMAGE_H

#include "../ElementState.h"
#include <SDL.h>

namespace m2::ui::element {
	struct ButtonStaticImageBlueprint {
		SDL_Rect texture_rect;
		SDL_Scancode keyboard_shortcut;
		int return_value;
	};

	struct ButtonStaticImageState : public ElementState {
		bool depressed;

        explicit ButtonStaticImageState(const ElementBlueprint* blueprint);
        // void draw() override;

        void set_depressed(bool state) override;
        int get_button_return_value() override;
        bool get_keyboard_shortcut_active(const uint8_t* raw_keyboard_state) const override;
	};
}

#endif //M2_BUTTONSTATICIMAGE_H
