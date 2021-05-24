#ifndef MAIN_H
#define MAIN_H

#include "Controls.h"
#include "Box2DWrapper.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))

int CurrentScreenWidth();
int CurrentScreenHeight();
float CurrentPixelsPerMeter();
SDL_Renderer* CurrentRenderer();
SDL_Texture* CurrentTextureLUT();
TTF_Font* CurrentFont();
Box2DWorld* CurrentWorld();

#endif
