#pragma once
#include "../Object.h"
#include "../Vec2i.h"
#include <SDL.h>

namespace m2::pedit {
	struct State {
		struct PaintMode {
			static void paint_color(const Vec2i& position);
		};
		struct EraseMode {
			static void erase_color(const Vec2i& position);
		};
		struct ColorPickerMode {
			static void pick_color(const Vec2i& position);
		};

		std::variant<std::monostate,PaintMode,EraseMode,ColorPickerMode> mode;
		SDL_Color selected_color;
		std::unordered_map<Vec2i, std::pair<Id,SDL_Color>, Vec2iHash> pixels;
		sdl::SurfaceUniquePtr image_surface;
		Vec2i image_offset;

		inline void select_color(const SDL_Color& color) { selected_color = color; }
		void deactivate_mode();
		void activate_paint_mode();
		void activate_erase_mode();
		void activate_color_picker_mode();
		static void save();
	};
}
