#ifndef M2_ELEMENTSTATE_H
#define M2_ELEMENTSTATE_H

#include "Action.h"
#include <m2/Event.hh>
#include <SDL_ttf.h>
#include <SDL.h>
#include <memory>

namespace m2::ui {
    enum TextAlignment {
        LEFT,
        CENTER,
        RIGHT
    };

    struct ElementBlueprint;
    struct ElementState {
        const ElementBlueprint* blueprint;
        SDL_Rect rect_px;

        explicit ElementState(const ElementBlueprint* blueprint);
		virtual ~ElementState() = default;
        virtual void update_position(const SDL_Rect& rect_px);
        virtual Action handle_events(const Events& events);
        virtual Action update_content();
        virtual void draw();

    protected:
        static SDL_Texture* generate_font_texture(const char* text);
        static void draw_text(const SDL_Rect& rect, SDL_Texture& texture, TextAlignment align);
    };
}

#endif //M2_ELEMENTSTATE_H
