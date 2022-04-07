#ifndef M2_UI_H
#define M2_UI_H

#include "ElementBlueprint.h"
#include "../Event.hh"
#include "../Def.h"
#include <SDL_ttf.h>
#include <SDL.h>
#include <functional>
#include <variant>
#include <optional>
#include <string>
#include <list>

namespace m2::ui {
	struct UIBlueprint {
		unsigned w, h; // unitless
		unsigned border_width_px;
		SDL_Color background_color;
		std::list<ElementBlueprint> elements;
	};

	struct UIState {
		const UIBlueprint* blueprint;
		SDL_Rect rect_px;
		std::list<std::unique_ptr<ElementState>> elements;

        UIState();
        explicit UIState(const UIBlueprint* blueprint);
        void update_positions(const SDL_Rect& rect);
        void update_contents();
        std::optional<int> handle_events(const Events& evs);
        void draw();

    private:
        ElementState* find_element_by_pixel(const m2::Vec2i& mouse_pos);
        ElementState* find_element_by_keyboard_shortcut(const uint8_t* raw_keyboard_state);
    public:
        static void draw_background_color(const SDL_Rect& rect, const SDL_Color& color);
        static void draw_border(const SDL_Rect& rect, unsigned border_width_px);
	};

    ValueOrM2Err<int> execute_blocking(const UIBlueprint* blueprint);
}

#endif //M2_UI_H
