#ifndef M2_STATICTEXT_H
#define M2_STATICTEXT_H

#include <SDL.h>
#include <string>

namespace m2::ui::element {
	struct StaticTextBlueprint {
		const std::string_view text;
	};

	struct StaticTextState {
		SDL_Texture* texture;
	};
}

#endif //M2_STATICTEXT_H
