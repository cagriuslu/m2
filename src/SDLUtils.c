#include "SDLUtils.h"

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

SDL_Rect SDLUtils_ShrinkRect(SDL_Rect rect, int xShrinkAmount, int yShrinkAmount) {
	return (SDL_Rect) { rect.x + xShrinkAmount, rect.y + yShrinkAmount, rect.w - 2 * xShrinkAmount, rect.h - 2 * yShrinkAmount };
}

SDL_Rect SDLUtils_ShrinkRect2(SDL_Rect rect, int top, int right, int bottom, int left) {
	return (SDL_Rect) { rect.x + left, rect.y + top, rect.w - left - right, rect.h - top - bottom };
}

SDL_Rect SDLUtils_SplitRect(SDL_Rect rect, unsigned horSplitCount, unsigned verSplitCount, unsigned horReturnIdx, unsigned horReturnLen, unsigned verReturnIdx, unsigned verReturnLen) {
	float horStep = (float)rect.w / (float)horSplitCount;
	float verStep = (float)rect.h / (float)verSplitCount;
	return (SDL_Rect) { rect.x + horStep * horReturnIdx, rect.y + verStep * verReturnIdx, horStep* horReturnLen, verStep* verReturnLen };
}
