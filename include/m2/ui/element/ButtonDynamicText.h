#ifndef M2_BUTTONDYNAMICTEXT_H
#define M2_BUTTONDYNAMICTEXT_H

#include "../ElementState.h"
#include <SDL.h>
#include <functional>
#include <string>

namespace m2::ui::element {
	struct ButtonDynamicTextBlueprint {
        std::string (*callback)();
		SDL_Scancode keyboard_shortcut;
		int return_value;
	};

	struct ButtonDynamicTextState : public ElementState {
        SDL_Texture* font_texture;
		bool depressed;

        explicit ButtonDynamicTextState(const ElementBlueprint* blueprint);
        void update_content() override;
        void draw() override;

        void set_depressed(bool state) override;
        int get_button_return_value() override;
        bool get_keyboard_shortcut_active(const uint8_t* raw_keyboard_state) const override;
	};
}

#endif //M2_BUTTONDYNAMICTEXT_H
