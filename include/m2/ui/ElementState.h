#ifndef M2_ELEMENTSTATE_H
#define M2_ELEMENTSTATE_H

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
        virtual void update_content();
        virtual void draw();

        virtual void set_depressed(bool state); // TODO rename to set_button_depressed
        virtual int get_button_return_value();
        virtual bool get_keyboard_shortcut_active(const uint8_t* raw_keyboard_state) const; // TODO rename

    protected:
        static SDL_Texture* generate_font_texture(const char* text);
        static void draw_text(const SDL_Rect& rect, SDL_Texture& texture);
        static bool check_key_pressed(const uint8_t* raw_keyboard_state, SDL_Scancode key);
    };
}

#endif //M2_ELEMENTSTATE_H
