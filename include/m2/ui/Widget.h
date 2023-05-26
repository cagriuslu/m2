#pragma once
#include "../Events.h"
#include "WidgetBlueprint.h"
#include "Action.h"
#include <optional>
#include <SDL.h>

namespace m2::ui {
	struct Widget {
		bool enabled{true};
		std::optional<float> disable_after; // TODO only implemented for Text
		const WidgetBlueprint* blueprint;
		SDL_Rect rect_px{};

		explicit Widget(const WidgetBlueprint* blueprint);
		virtual ~Widget() = default;
		virtual void update_position(const SDL_Rect& rect_px);
		virtual Action handle_events(Events& events);
		virtual Action update_content();
		virtual void draw();

		// Helpers
		static void draw_background_color(const SDL_Rect& rect, const SDL_Color& color);
		static void draw_text(const SDL_Rect& rect, SDL_Texture& texture, TextAlignment align);
		static void draw_border(const SDL_Rect& rect, unsigned border_width_px, const SDL_Color& color = {255, 255, 255, 255});
	};
}
