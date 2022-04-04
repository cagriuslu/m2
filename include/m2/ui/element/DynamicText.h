#ifndef M2_DYNAMICTEXT_H
#define M2_DYNAMICTEXT_H

#include "../ElementState.h"
#include <SDL.h>
#include <functional>
#include <string>

namespace m2::ui::element {
	struct DynamicTextBlueprint {
        std::string (*callback)();
	};

	struct DynamicTextState : public ElementState {
        SDL_Texture* font_texture;

        explicit DynamicTextState(const ElementBlueprint* blueprint);
        void update_content() override;
        void draw() override;
	};
}

#endif //M2_DYNAMICTEXT_H
