#ifndef MAIN_H
#define MAIN_H

#include "Controls.h"
#include "Box2D.h"
#include "Array.h"
#include "Event.h"
#include "Level.h"
#include "Character.h"
#include "TextureMap.h"
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
#define TILE_HEIGHT (24)
#define TILE_HEIGHT_STR "24"
#define TEXTURE_FILE_KEY TILE_WIDTH_STR "x" TILE_HEIGHT_STR

#define GAME_ASPECT_RATIO (5.0f / 4.0f)
#define GAME_AND_HUD_ASPECT_RATIO (16.0f / 9.0f)
#define HUD_ASPECT_RATIO ((GAME_AND_HUD_ASPECT_RATIO - GAME_ASPECT_RATIO) / 2.0f)

int CurrentScreenWidth();
int CurrentScreenHeight();
float CurrentPixelsPerMeter();
int CurrentTileWidth();
uint32_t CurrentWindowPixelFormat();
SDL_Renderer* CurrentRenderer();
SDL_Texture* CurrentTextureLUT();
TTF_Font* CurrentFont();
TextureMap* CurrentTextureMap();

Level* CurrentLevel();
Events* CurrentEvents();
unsigned DeltaTicks();

Vec2F CurrentPointerPositionInWorld(void);

#endif
