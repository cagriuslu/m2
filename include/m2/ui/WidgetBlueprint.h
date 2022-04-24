#ifndef M2_WIDGETBLUEPRINT_H
#define M2_WIDGETBLUEPRINT_H

#include "widget/Image.h"
#include "widget/NestedUI.h"
#include "widget/ProgressBar.h"
#include "widget/Text.h"
#include "widget/TextInput.h"
#include <SDL_ttf.h>
#include <SDL.h>
#include <memory>
#include <variant>

namespace m2::ui {
    struct WidgetState;
    struct WidgetBlueprint {
        unsigned x{}, y{}, w{}, h{}; // unitless
        unsigned border_width_px{};
        SDL_Color background_color{};

        using WidgetBlueprintVariant = std::variant<
                wdg::NestedUIBlueprint,
                wdg::ImageBlueprint,
				wdg::ProgressBarBlueprint,
                wdg::TextBlueprint,
                wdg::TextInputBlueprint>;
        WidgetBlueprintVariant variant;

        [[nodiscard]] std::unique_ptr<WidgetState> get_state() const;
    };
}

#endif //M2_WIDGETBLUEPRINT_H
