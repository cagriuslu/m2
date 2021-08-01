#ifndef MAIN_H
#define MAIN_H

#include "Controls.h"
#include "Box2D.h"
#include "Array.h"
#include "Level.h"
#include "Character.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))

#define TILE_WIDTH (24)
#define TILE_WIDTH_STR "24"

int CurrentScreenWidth();
int CurrentScreenHeight();
float CurrentPixelsPerMeter();
int CurrentTileWidth();
uint32_t CurrentWindowPixelFormat();
SDL_Renderer* CurrentRenderer();
SDL_Texture* CurrentTextureLUT();
TTF_Font* CurrentFont();

Level* CurrentLevel();
unsigned DeltaTicks();

Vec2F CurrentPointerPositionInWorld();

#endif
