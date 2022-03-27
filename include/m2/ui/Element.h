#ifndef M2_ELEMENT_H
#define M2_ELEMENT_H

#include "element/ButtonDynamicImage.h"
#include "element/ButtonDynamicText.h"
#include "element/ButtonStaticImage.h"
#include "element/ButtonStaticText.h"
#include "element/DynamicImage.h"
#include "element/DynamicText.h"
#include "element/NestedUI.h"
#include "element/StaticImage.h"
#include "element/StaticText.h"
#include <SDL.h>
#include <variant>

namespace m2::ui {
	using namespace element;

	using ElementBlueprintVariant = std::variant<
			NestedUIBlueprint,
			StaticTextBlueprint,
			StaticImageBlueprint,
			DynamicTextBlueprint,
			DynamicImageBlueprint,
			ButtonStaticTextBlueprint,
			ButtonStaticImageBlueprint,
			ButtonDynamicTextBlueprint,
			ButtonDynamicImageBlueprint>;

	struct ElementBlueprint {
		const unsigned x, y, w, h; // unitless
		const unsigned border_width_px;
		const SDL_Color background_color;
		const ElementBlueprintVariant blueprint_variant;
	};

	using ElementStateVariant = std::variant<
			NestedUIState,
			StaticTextState,
			StaticImageState,
			DynamicTextState,
			DynamicImageState,
			ButtonStaticTextState,
			ButtonStaticImageState,
			ButtonDynamicTextState,
			ButtonDynamicImageState>;

	struct ElementState {
		const ElementBlueprint& blueprint;
		SDL_Rect rect;
		ElementStateVariant state_variant;
	};
}

#endif //M2_ELEMENT_H
