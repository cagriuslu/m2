#include "SDLUtils.h"

#include <stdio.h>

Vec2I SDLUtils_CenterOfRect(SDL_Rect rect) {
	return (Vec2I) { rect.x + rect.w / 2, rect.y + rect.h / 2 };
}

SDL_Cursor* SDLUtils_CreateCursor() {
    const char* str =
		"                                "
		"            X     X             "
		"           X.X   X.X            "
		"           X..X X..X            "
		"           X...X...X            "
		"            X.....X             "
		"            X.....X             "
		"             X...X              "
		"             X...X              "
		"              X.X               "
		"              X.X               "
		"  XXX          X          XXX   "
		" X...XX                 XX...X  "
		"  X....XX             XX....X   "
		"   X.....XX   XXX   XX.....X    "
		"    X......X  X.X  X......X     "
		"   X.....XX   XXX   XX.....X    "
		"  X....XX             XX....X   "
		" X...XX                 XX...X  "
		"  XXX          X          XXX   "
		"              X.X               "
		"              X.X               "
		"             X...X              "
		"             X...X              "
		"            X.....X             "
		"            X.....X             "
		"           X...X...X            "
		"           X..X X..X            "
		"           X.X   X.X            "
		"            X     X             "
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
