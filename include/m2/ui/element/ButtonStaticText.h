#ifndef M2_BUTTONSTATICTEXT_H
#define M2_BUTTONSTATICTEXT_H

#include <SDL.h>
#include <string>

namespace m2::ui::element {
	struct ButtonStaticTextBlueprint {
		const std::string_view text;
		const SDL_Scancode keyboard_shortcut;
		const int return_value;
	};

	struct ButtonStaticTextState {
		SDL_Texture* texture;
		bool depressed;
	};
}

#endif //M2_BUTTONSTATICTEXT_H
