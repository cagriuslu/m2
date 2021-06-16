#ifndef TILE_LOOKUP_H
#define TILE_LOOKUP_H

#include "Vec2I.h"
#include "Vec2F.h"

typedef struct _TileDef {
	Vec2I txIndex;
	Vec2F colliderSize;
	Vec2F colliderOffset;
} TileDef;

TileDef TileLookup(const char* tileName);

#endif
