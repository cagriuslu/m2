#ifndef M2_UI_H
#define M2_UI_H

#include "WidgetBlueprint.h"
#include "Action.h"
#include "../Events.h"
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
		std::list<WidgetBlueprint> widgets;
	};

	struct UIState {
		const UIBlueprint* blueprint;
		SDL_Rect rect_px;
		std::list<std::unique_ptr<WidgetState>> widgets;

        UIState();
        explicit UIState(const UIBlueprint* blueprint);
        void update_positions(const SDL_Rect& rect);
        Action handle_events(Events& events);
        Action update_contents();
        void draw();

    public:
        static void draw_background_color(const SDL_Rect& rect, const SDL_Color& color);
        static void draw_border(const SDL_Rect& rect, unsigned border_width_px);
	};

    Action execute_blocking(const UIBlueprint* blueprint);
	Action execute_blocking(const UIBlueprint* blueprint, SDL_Rect rect);

    extern const UIBlueprint console_ui;
}

#endif //M2_UI_H
