#ifndef MAIN_H
#define MAIN_H

#include "Controls.h"
#include "Box2D.h"
#include "Array.h"
#include "Event.h"
#include "Level.h"
#include "Window.h"
#include "Character.h"
#include "TextureMap.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

#define TILE_WIDTH (24)
#define TILE_WIDTH_STR "24"
#define TILE_HEIGHT (24)
#define TILE_HEIGHT_STR "24"
#define TEXTURE_FILE_KEY TILE_WIDTH_STR "x" TILE_HEIGHT_STR

Window* CurrentWindow();
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
