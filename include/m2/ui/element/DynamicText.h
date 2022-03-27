#ifndef M2_DYNAMICTEXT_H
#define M2_DYNAMICTEXT_H

#include <SDL.h>
#include <functional>
#include <string>

namespace m2::ui::element {
	struct DynamicTextBlueprint {
		const std::function<std::string()> callback;
	};

	struct DynamicTextState {
		SDL_Texture* texture;
	};
}

#endif //M2_DYNAMICTEXT_H
