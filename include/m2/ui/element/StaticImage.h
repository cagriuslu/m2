#ifndef M2_STATICIMAGE_H
#define M2_STATICIMAGE_H

#include <SDL.h>

namespace m2::ui::element {
	struct StaticImageBlueprint {
		const SDL_Rect texture_rect;
	};

	struct StaticImageState {
		int _dummy;
	};
}

#endif //M2_STATICIMAGE_H
