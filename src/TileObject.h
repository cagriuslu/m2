#ifndef TILE_OBJECT_H
#define TILE_OBJECT_H

#include "Vec2I.h"
#include "TileLookup.h"
#include "Box2DWrapper.h"
#include "Object.h"
#include <SDL.h>

typedef struct _TileObject {
	Object super;
	Vec2I pos;
	SDL_Rect txSrc;
	Box2DBody* body;
} TileObject;

int TileInit(TileObject* tile, Vec2I position, Vec2I txIndex, Vec2F colliderSize);
void TileDeinit(TileObject* tile);

int NewTileInit(Object* obj, TileDef tileDef, Vec2F position);

#endif
