#ifndef M2_BUTTONDYNAMICTEXT_H
#define M2_BUTTONDYNAMICTEXT_H

#include <SDL.h>
#include <functional>
#include <string>

namespace m2::ui::element {
	struct ButtonDynamicTextBlueprint {
		const std::function<std::string()> callback;
		const SDL_Scancode keyboard_shortcut;
		const int return_value;
	};

	struct ButtonDynamicTextState {
		SDL_Texture* texture;
		bool depressed;
	};
}

#endif //M2_BUTTONDYNAMICTEXT_H
