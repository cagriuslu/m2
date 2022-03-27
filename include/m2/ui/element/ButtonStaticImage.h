#ifndef M2_BUTTONSTATICIMAGE_H
#define M2_BUTTONSTATICIMAGE_H

#include <SDL.h>

namespace m2::ui::element {
	struct ButtonStaticImageBlueprint {
		const SDL_Rect texture_rect;
		const SDL_Scancode keyboard_shortcut;
		const int return_value;
	};

	struct ButtonStaticImageState {
		bool depressed;
	};
}

#endif //M2_BUTTONSTATICIMAGE_H
