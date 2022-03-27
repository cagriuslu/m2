#ifndef M2_BUTTONDYNAMICIMAGE_H
#define M2_BUTTONDYNAMICIMAGE_H

#include <SDL.h>
#include <functional>

namespace m2::ui::element {
	struct ButtonDynamicImageBlueprint {
		const std::function<SDL_Rect()> callback;
		const SDL_Scancode keyboard_shortcut;
		const int return_value;
	};

	struct ButtonDynamicImageState {
		bool depressed;
		SDL_Rect texture_rect;
	};
}

#endif //M2_BUTTONDYNAMICIMAGE_H
