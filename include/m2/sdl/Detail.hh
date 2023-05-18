#ifndef M2_SDL_DETAIL_H
#define M2_SDL_DETAIL_H

#include <Rect2i.pb.h>
#include <SDL2/SDL.h>
#include <string_view>

SDL_Cursor* SdlUtils_CreateCursor();

namespace m2::sdl {
	struct TextureDeleter {
		void operator()(SDL_Texture* t);
	};

	using TextureUniquePtr = std::unique_ptr<SDL_Texture, TextureDeleter>;

	struct SurfaceDeleter {
		void operator()(SDL_Surface* s);
	};

	using SurfaceUniquePtr = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

	using ticks_t = int64_t;
	void delay(ticks_t duration);
	ticks_t get_ticks();
	ticks_t get_ticks_since(ticks_t last_ticks, ticks_t pause_ticks = 0, ticks_t min = 0);

	int get_refresh_rate();

	int draw_circle(SDL_Renderer* renderer, SDL_Color color, SDL_Rect* dst_rect, unsigned piece_count);

	TextureUniquePtr generate_font(const std::string& text, SDL_Color color = {255, 255, 255, 255});

	/// Assumes surface is already locked
	void set_pixel(SDL_Surface* surface, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	void set_pixel(SDL_Surface* surface, int x, int y, uint32_t pixel);
	uint32_t get_pixel(const SDL_Surface* surface, int x, int y);

	SDL_Rect expand_rect(const SDL_Rect& rect, int diff); // Replace with Rect2i::expand
	SDL_Rect to_rect(const pb::Rect2i& pb_rect); // Replace with Rect2i
}

#endif //M2_SDL_DETAIL_H
