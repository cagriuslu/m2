#ifndef M2_STATICIMAGE_H
#define M2_STATICIMAGE_H

#include "../ElementState.h"
#include <SDL.h>

namespace m2::ui::element {
	struct StaticImageBlueprint {
		SDL_Rect texture_rect;
	};

	struct StaticImageState : public ElementState {

        explicit StaticImageState(const ElementBlueprint* blueprint);
        // void draw() override;
	};
}

#endif //M2_STATICIMAGE_H
