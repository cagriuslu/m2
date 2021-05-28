#ifndef TILE_H
#define TILE_H

#include "Vec2I.h"
#include "Box2DWrapper.h"
#include <SDL.h>

typedef struct _Tile {
	Vec2F pos;
	SDL_Rect txSrc;
	Box2DBody* body;
} Tile;

int TileInit(Tile* tile, Vec2F position, Vec2I txIndex);
void TileDeinit(Tile* tile);

#endif
