#pragma once
#include "../Meta.h"
#include "../VecI.h"
#include <RectI.pb.h>
#include "../RectI.h"
#include <SDL2/SDL.h>

SDL_Cursor* SdlUtils_CreateCursor();

namespace m2::sdl {
	using ticks_t = int64_t;
	void delay(ticks_t duration);
	ticks_t get_ticks();
	ticks_t get_ticks_since(ticks_t last_ticks, ticks_t pause_ticks = 0);

	int get_refresh_rate();
	VecI mouse_position();

	int draw_circle(SDL_Renderer* renderer, SDL_Color color, SDL_Rect* dst_rect, unsigned piece_count);
	int draw_disk(SDL_Renderer* renderer, const VecF& center_position_px, const SDL_Color& center_color, float radius_px, const SDL_Color& edge_color, unsigned steps = 96);

	/// Assumes surface is already locked
	void set_pixel(SDL_Surface* surface, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	void set_pixel(SDL_Surface* surface, int x, int y, uint32_t pixel);
	/// Returns the BGRA32 pixel if x,y are in bounds. Returns nullopt if not.
	/// Assumes surface is already locked
	std::optional<uint32_t> get_pixel(const SDL_Surface* surface, int x, int y);
}
