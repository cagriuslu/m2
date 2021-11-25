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

SDL_Texture* CurrentTextureLUT();
TextureMap* CurrentTextureMap();

Level* CurrentLevel();

Vec2F CurrentPointerPositionInWorld();

#endif
