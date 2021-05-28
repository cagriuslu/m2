#ifndef MAIN_H
#define MAIN_H

#include "Controls.h"
#include "Box2DWrapper.h"
#include "Array.h"
#include "DrawList.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))

int CurrentScreenWidth();
int CurrentScreenHeight();
float CurrentPixelsPerMeter();
int CurrentTileWidth();
uint32_t CurrentWindowPixelFormat();
SDL_Renderer* CurrentRenderer();
SDL_Texture* CurrentTextureLUT();
TTF_Font* CurrentFont();

Box2DWorld* CurrentWorld();
Array* CurrentObjectArray();
DrawList* CurrentDrawList();

#endif
