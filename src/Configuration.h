#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "Vec2F.h"
#include <SDL.h>

#define CFG_TILE_SIZE (24)

typedef enum _ColliderType {
	COLLIDER_TYPE_NONE = 0,
	COLLIDER_TYPE_RECTANGLE,
	COLLIDER_TYPE_CIRCLE
} ColliderType;
typedef struct _ColliderConfiguration {
	ColliderType colliderType;
	union {
		RectangleColliderDef rectangle;
		CircleColliderDef circle;
	} colliderUnion;
} ColliderConfiguration;

typedef struct _ObjectConfiguration {
	SDL_Rect textureRect;
	Vec2F objectCenter_px; // wrt the center of textureRect
	Vec2F objectCenter_m; // wrt the center of textureRect

} ObjectConfiguration;

typedef struct _GroundTileConfiguration {

} GroundTileConfiguration;

#endif
