#ifndef M2_BUTTONDYNAMICIMAGE_H
#define M2_BUTTONDYNAMICIMAGE_H

#include "../ElementState.h"
#include <SDL.h>
#include <functional>

namespace m2::ui::element {
    struct ButtonDynamicImageBlueprint {
        SDL_Rect (*callback)();
		SDL_Scancode keyboard_shortcut;
		int return_value;
	};

	struct ButtonDynamicImageState : public ElementState {
		bool depressed;
		SDL_Rect texture_rect;

        explicit ButtonDynamicImageState(const ElementBlueprint* blueprint);
        void update_content() override;
        // void draw() override;

        void set_depressed(bool state) override;
        int get_button_return_value() override;
        bool get_keyboard_shortcut_active(const uint8_t* raw_keyboard_state) const override;
	};
}

#endif //M2_BUTTONDYNAMICIMAGE_H
