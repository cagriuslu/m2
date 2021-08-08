#ifndef SDL_UTILS_H
#define SDL_UTILS_H

#include "Vec2I.h";
#include <SDL.h>

Vec2I SDLUtils_CenterOfRect(SDL_Rect rect);

SDL_Cursor* SDLUtils_CreateCursor();

#endif
