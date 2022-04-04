#ifndef M2_STATICTEXT_H
#define M2_STATICTEXT_H

#include "../ElementState.h"
#include <SDL.h>
#include <string>

namespace m2::ui::element {
	struct StaticTextBlueprint {
		std::string_view text;
	};

	struct StaticTextState : public ElementState {
        SDL_Texture* font_texture;

        explicit StaticTextState(const ElementBlueprint* blueprint);
        void update_content() override;
        void draw() override;
	};
}

#endif //M2_STATICTEXT_H
