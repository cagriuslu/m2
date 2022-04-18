#ifndef M2_ELEMENTBLUEPRINT_H
#define M2_ELEMENTBLUEPRINT_H

#include "element/Image.h"
#include "element/Text.h"
#include "element/NestedUI.h"
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
                element::TextBlueprint,
                element::ImageBlueprint>;
        ElementBlueprintVariant variant;

        [[nodiscard]] std::unique_ptr<ElementState> get_state() const;
    };
}

#endif //M2_ELEMENTBLUEPRINT_H
