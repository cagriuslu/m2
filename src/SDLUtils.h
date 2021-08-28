#ifndef SDL_UTILS_H
#define SDL_UTILS_H

#include "Vec2I.h"
#include <SDL.h>

Vec2I SDLUtils_CenterOfRect(SDL_Rect rect);

SDL_Cursor* SDLUtils_CreateCursor();

SDL_Rect SDLUtils_ShrinkRect(SDL_Rect rect, int xShrinkAmount, int yShrinkAmount);
SDL_Rect SDLUtils_ShrinkRect2(SDL_Rect rect, int top, int right, int bottom, int left);
SDL_Rect SDLUtils_SplitRect(SDL_Rect rect, unsigned horSplitCount, unsigned verSplitCount, unsigned horReturnIdx, unsigned horReturnLen, unsigned verReturnIdx, unsigned verReturnLen);

#endif
