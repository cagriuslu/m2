#ifndef SDL_UTILS_H
#define SDL_UTILS_H

#include <SDL.h>

SDL_Cursor* SDLUtils_CreateCursor();

uint32_t SDLUtils_GetTicksAtLeast1ms(uint32_t lastTicks);

#endif