#ifndef TILESET_H
#define TILESET_H

#include "VSON.h"
#include "HashMap.h"
#include "Vec2F.h"
#include <SDL.h>

typedef enum _ColliderType {
	COLLIDER_TYPE_NONE = 0,
	COLLIDER_TYPE_RECTANGLE,
	COLLIDER_TYPE_CIRCLE
} ColliderType;

typedef struct _RectangleColliderDef {
	Vec2F center; // wrt object center
	Vec2F dims;
} RectangleColliderDef;

typedef struct _CircleColliderDef {
	Vec2F center; // wrt object center
	float radius;
} CircleColliderDef;

typedef struct _ColliderDef {
	ColliderType colliderType;
	union _ColliderUnion {
		RectangleColliderDef rectangle;
		CircleColliderDef circle;
	} colliderUnion;
} ColliderDef;

typedef struct _TerrainDef {
	SDL_Rect textureRect;
	ColliderDef colliderDef;
} TerrainDef;

typedef struct _ObjectDef {
	SDL_Rect textureRect;
	Vec2F center; // wrt center of the texture
	ColliderDef colliderDef;
} ObjectDef;

typedef struct _TileSet {
	HashMap terrainDefs;
	HashMap objectDefs;
	SDL_Texture* texture;
	int tileWidth;
} TileSet;

XErr TileSet_InitFromFile(TileSet* ts, const char* fpath);
XErr TileSet_InitFromVson(TileSet* ts, VSON* vson);

void TileSet_Term(TileSet* ts);

#endif
