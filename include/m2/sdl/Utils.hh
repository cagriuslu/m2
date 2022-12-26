#ifndef SDL_UTILS_H
#define SDL_UTILS_H

#include <Rect2i.pb.h>
#include <SDL2/SDL.h>

SDL_Cursor* SdlUtils_CreateCursor();

namespace m2::sdl {
	struct TextureDeleter {
		void operator()(SDL_Texture* t);
	};

	struct SurfaceDeleter {
		void operator()(SDL_Surface* s);
	};

	uint32_t get_ticks(uint32_t last_ticks, uint32_t pause_ticks, uint32_t min = 0);

	/// Assumes surface is already locked
	void set_pixel(SDL_Surface* surface, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	void set_pixel(SDL_Surface* surface, int x, int y, uint32_t pixel);

	SDL_Rect expand_rect(const SDL_Rect& rect, int diff); // Replace with Rect2i::expand
	SDL_Rect to_rect(const pb::Rect2i& pb_rect); // Replace with Rect2i
}

#endif
