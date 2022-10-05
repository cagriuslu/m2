#include "m2/SdlUtils.hh"

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

uint32_t SdlUtils_GetTicksAtLeast1ms(uint32_t lastTicks, uint32_t nongame_ticks) {
	uint32_t ticks = SDL_GetTicks() - nongame_ticks;
	while (ticks == lastTicks) {
		SDL_Delay(1);
		ticks = SDL_GetTicks() - nongame_ticks;
	}
	return ticks;
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

void m2::SdlTextureDeleter::operator()(SDL_Texture *t) {
	SDL_DestroyTexture(t);
}

void m2::SdlSurfaceDeleter::operator()(SDL_Surface *s) {
	SDL_FreeSurface(s);
}
