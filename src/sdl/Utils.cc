#include <m2/sdl/Utils.hh>
#include <m2/Game.h>

SDL_Cursor* SdlUtils_CreateCursor() {
    const char* str =
		"                                "
		"                                "
		"             XXXXX              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             XXXXX              "
		"                                "
		"  XXXXXXXXXX XXXXX XXXXXXXXXX   "
		"  X........X X...X X........X   "
		"  X........X X...X X........X   "
		"  X........X X...X X........X   "
		"  XXXXXXXXXX XXXXX XXXXXXXXXX   "
		"                                "
		"             XXXXX              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             XXXXX              "
		"                                "
		"                                "
		"                                "
	;

    uint8_t data[128] = { 0 };
    uint8_t mask[128] = { 0 };
    const unsigned side_size = 32;

	for (unsigned y = 0; y < side_size; y++) {
		for (unsigned x = 0; x < side_size; x++) {
            unsigned index = y * side_size + x;
            char c = str[index];
			if (c == 'X') {
                data[index / 8] |= 0x80 >> (index % 8);
                mask[index / 8] |= 0x80 >> (index % 8);
			} else if (c == '.') {
                mask[index / 8] |= 0x80 >> (index % 8);
			}
		}
	}

	return SDL_CreateCursor(data, mask, side_size, side_size, side_size / 2 - 1, side_size / 2 - 1);
}

void m2::sdl::delay(ticks_t duration) {
	if (0 < duration) {
		SDL_Delay(duration);
	}
}

m2::sdl::ticks_t m2::sdl::get_ticks(ticks_t pause_ticks) {
	return static_cast<int64_t>(SDL_GetTicks64()) - pause_ticks;
}

m2::sdl::ticks_t m2::sdl::get_ticks(ticks_t last_ticks, ticks_t pause_ticks, ticks_t min) {
	auto ticks = get_ticks(pause_ticks);
	if (min) {
		while (ticks <= last_ticks + min) {
			delay(min);
			ticks += min;
		}
	}
	return ticks;
}

int m2::sdl::get_refresh_rate() {
	SDL_DisplayMode dm{};
	SDL_GetWindowDisplayMode(GAME.sdlWindow, &dm);
	return dm.refresh_rate;
}

void m2::sdl::set_pixel(SDL_Surface* surface, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	set_pixel(surface, x, y, SDL_MapRGBA(surface->format, r, g, b, a));
}
void m2::sdl::set_pixel(SDL_Surface* surface, int x, int y, uint32_t pixel) {
	auto* target_pixel = (uint32_t*)((uint8_t*)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel);
	*target_pixel = pixel;
}
uint32_t m2::sdl::get_pixel(const SDL_Surface* surface, int x, int y) {
	if (surface->format->format != SDL_PIXELFORMAT_BGRA32) {
		throw M2FATAL("get_pixel is called with a surface with an unsupported pixel format");
	}
	auto* target_pixel = (uint32_t*)((uint8_t*)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel);
	return *target_pixel;
}

SDL_Rect m2::sdl::expand_rect(const SDL_Rect& rect, int diff) {
	return {
		rect.x - diff,
		rect.y - diff,
		rect.w + 2 * diff,
		rect.h + 2 * diff
	};
}

SDL_Rect m2::sdl::to_rect(const pb::Rect2i& pb_rect) {
	return {pb_rect.x(), pb_rect.y(), pb_rect.w(), pb_rect.h()};
}

void m2::sdl::TextureDeleter::operator()(SDL_Texture *t) {
	SDL_DestroyTexture(t);
}

void m2::sdl::SurfaceDeleter::operator()(SDL_Surface *s) {
	SDL_FreeSurface(s);
}
