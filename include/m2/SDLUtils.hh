#ifndef SDL_UTILS_H
#define SDL_UTILS_H

#include <SDL.h>

SDL_Cursor* SDLUtils_CreateCursor();

uint32_t SDLUtils_GetTicksAtLeast1ms(uint32_t lastTicks, uint32_t nongame_ticks);

namespace m2 {
	SDL_Rect expand(const SDL_Rect& rect, int diff);
}

#endif
