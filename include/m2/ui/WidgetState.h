#ifndef M2_WIDGETSTATE_H
#define M2_WIDGETSTATE_H

#include "Action.h"
#include <m2/Events.h>
#include <SDL_ttf.h>
#include <SDL.h>
#include <memory>

namespace m2::ui {
    enum TextAlignment {
        CENTER [[maybe_unused]],
        LEFT,
        RIGHT
    };

    struct WidgetBlueprint;
    struct WidgetState {
        const WidgetBlueprint* blueprint;
        SDL_Rect rect_px;

        explicit WidgetState(const WidgetBlueprint* blueprint);
		virtual ~WidgetState() = default;
        virtual void update_position(const SDL_Rect& rect_px);
        virtual Action handle_events(Events& events);
        virtual Action update_content();
        virtual void draw();

    protected:
        static SDL_Texture* generate_font_texture(const char* text);
        static void draw_text(const SDL_Rect& rect, SDL_Texture& texture, TextAlignment align);
    };
}

#endif //M2_WIDGETSTATE_H
