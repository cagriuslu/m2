#pragma once
#include "../Object.h"
#include "../math/VecI.h"
#include <SDL.h>

namespace m2::pixel_editor {
	struct State {
		struct PaintMode {
			static void paint_color(const VecI& position);
		};
		struct EraseMode {
			static void erase_color(const VecI& position);
		};
		struct ColorPickerMode {
			static void pick_color(const VecI& position);
		};

		std::variant<std::monostate,PaintMode,EraseMode,ColorPickerMode> mode;
		SDL_Color selected_color;
		std::unordered_map<VecI, std::pair<Id,SDL_Color>, VecIHash> pixels;
		sdl::SurfaceUniquePtr image_surface;
		VecI image_offset;

		inline void select_color(const SDL_Color& color) { selected_color = color; }
		void deactivate_mode();
		void activate_paint_mode();
		void activate_erase_mode();
		void activate_color_picker_mode();
		static void save();
	};
}
