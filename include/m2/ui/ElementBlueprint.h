#ifndef M2_ELEMENTBLUEPRINT_H
#define M2_ELEMENTBLUEPRINT_H

#include "element/ButtonDynamicImage.h"
#include "element/ButtonDynamicText.h"
#include "element/ButtonStaticImage.h"
#include "element/ButtonStaticText.h"
#include "element/DynamicImage.h"
#include "element/DynamicText.h"
#include "element/NestedUI.h"
#include "element/StaticImage.h"
#include "element/StaticText.h"
#include <SDL_ttf.h>
#include <SDL.h>
#include <memory>
#include <variant>

namespace m2::ui {
    struct ElementState;

    struct ElementBlueprint {
        unsigned x, y, w, h; // unitless
        unsigned border_width_px;
        SDL_Color background_color;

        using ElementBlueprintVariant = std::variant<
                element::NestedUIBlueprint,
                element::StaticTextBlueprint,
                element::StaticImageBlueprint,
                element::DynamicTextBlueprint,
                element::DynamicImageBlueprint,
                element::ButtonStaticTextBlueprint,
                element::ButtonStaticImageBlueprint,
                element::ButtonDynamicTextBlueprint,
                element::ButtonDynamicImageBlueprint>;
        ElementBlueprintVariant blueprint_variant;

        std::unique_ptr<ElementState> get_state() const;
    };
}

#endif //M2_ELEMENTBLUEPRINT_H
