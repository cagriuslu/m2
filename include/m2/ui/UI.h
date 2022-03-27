#ifndef M2_UI_H
#define M2_UI_H

#include "Element.h"
#include <SDL.h>
#include <functional>
#include <variant>
#include <string>
#include <list>

namespace m2::ui {
	struct Blueprint {
		const unsigned w, h; // unitless
		const unsigned border_width_px;
		const SDL_Color background_color;
		const std::list<ElementBlueprint> elements;
	};

	struct State {
		const Blueprint& blueprint;
		SDL_Rect rect;
		std::list<ElementState> elements;
	};
}

#endif //M2_UI_H
