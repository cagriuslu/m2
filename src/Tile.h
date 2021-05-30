#ifndef TILE_H
#define TILE_H

#include "Vec2I.h"
#include "Box2DWrapper.h"
#include <SDL.h>

typedef struct _Tile {
	Vec2I pos;
	SDL_Rect txSrc;
	Box2DBody* body;
} Tile;

int TileInit(Tile* tile, Vec2I position, Vec2I txIndex, Vec2F colliderSize);
void TileDeinit(Tile* tile);

typedef struct _TileDef {
	Vec2I txIndex;
	Vec2F colliderSize;
} TileDef;

TileDef TileLookup(const char* tileName);

#endif
