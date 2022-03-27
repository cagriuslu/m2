#ifndef M2_DYNAMICIMAGE_H
#define M2_DYNAMICIMAGE_H

#include <SDL.h>
#include <functional>

namespace m2::ui::element {
	struct DynamicImageBlueprint {
		const std::function<SDL_Rect()> callback;
	};

	struct DynamicImageState {
		SDL_Rect texture_rect;
	};
}

#endif //M2_DYNAMICIMAGE_H
