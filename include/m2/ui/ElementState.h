#ifndef M2_ELEMENTSTATE_H
#define M2_ELEMENTSTATE_H

#include "Action.h"
#include <SDL_ttf.h>
#include <SDL.h>
#include <memory>

namespace m2::ui {
    struct ElementBlueprint;

    struct ElementState {
        const ElementBlueprint* blueprint;
        SDL_Rect rect_px;

        explicit ElementState(const ElementBlueprint* blueprint);
        virtual void update_position(const SDL_Rect& rect_px);
        virtual Action update_content();
        virtual void draw();

		virtual SDL_Scancode get_keyboard_shortcut() const;
        virtual void set_depressed(bool state);
		virtual Action action();

    protected:
        static SDL_Texture* generate_font_texture(const char* text);
        static void draw_text(const SDL_Rect& rect, SDL_Texture& texture);
    };
}

#endif //M2_ELEMENTSTATE_H
