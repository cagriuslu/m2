#ifndef MAIN_H
#define MAIN_H

#include "Controls.h"
#include "Box2D.h"
#include "Game.h"
#include "Array.h"
#include "Event.h"
#include "Level.h"
#include "Character.h"
#include "TextureMap.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

Game* CurrentGame();

SDL_Renderer* CurrentRenderer();
SDL_Texture* CurrentTextureLUT();
TTF_Font* CurrentFont();
TextureMap* CurrentTextureMap();

Level* CurrentLevel();
Events* CurrentEvents();
unsigned DeltaTicks();

Vec2F CurrentPointerPositionInWorld();

#endif
