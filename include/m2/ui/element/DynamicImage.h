#ifndef M2_DYNAMICIMAGE_H
#define M2_DYNAMICIMAGE_H

#include "../ElementState.h"
#include <SDL.h>
#include <functional>

namespace m2::ui::element {
	struct DynamicImageBlueprint {
        SDL_Rect (*callback)();
	};

	struct DynamicImageState : public ElementState {
		SDL_Rect texture_rect;

        explicit DynamicImageState(const ElementBlueprint* blueprint);
        void update_content() override;
        // void draw() override;
	};
}

#endif //M2_DYNAMICIMAGE_H
