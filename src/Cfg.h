#ifndef CFG_H
#define CFG_H

#include "Vec2F.h"
#include <SDL.h>

#define CFG_TILE_SIZE (24)
#define CFG_TEXTURE_FILE "resources/24.png"

typedef struct _CfgCollider {
	enum {
		COLLIDER_TYPE_NONE = 0,
		COLLIDER_TYPE_RECTANGLE,
		COLLIDER_TYPE_CIRCLE
	} type;
	union {
		struct {
			Vec2F center_px;
			Vec2F center_m;
			Vec2F dims_px;
			Vec2F dims_m;
		} rect;
		struct {
			Vec2F center_px;
			Vec2F center_m;
			float radius_px;
			float radius_m;
		} circ;
	} colliderUnion;
} CfgCollider;

typedef struct _CfgGroundTile {
	SDL_Rect textureRect;
	CfgCollider collider;
} CfgGroundTile;

extern const CfgGroundTile CFG_GNDTILE_DEFAULT;
extern const CfgGroundTile CFG_GNDTILE_GROUND000;
extern const CfgGroundTile CFG_GNDTILE_CLIFF000T;
extern const CfgGroundTile CFG_GNDTILE_CLIFF000R;
extern const CfgGroundTile CFG_GNDTILE_CLIFF000B;
extern const CfgGroundTile CFG_GNDTILE_CLIFF000L;
extern const CfgGroundTile CFG_GNDTILE_CLIFF000TR;
extern const CfgGroundTile CFG_GNDTILE_CLIFF000TL;
extern const CfgGroundTile CFG_GNDTILE_CLIFF000BR;
extern const CfgGroundTile CFG_GNDTILE_CLIFF000BL;

#define CFG_OBJ_ID_PLAYER (1)
#define CFG_OBJ_ID_ENEMY  (2)

typedef struct _CfgObject {
	int id;
	SDL_Rect textureRect;
	Vec2F objCenter_px;
	Vec2F objCenter_m;
	CfgCollider collider;
} CfgObject;

extern const CfgObject CFG_OBJ_BOX000;
extern const CfgObject CFG_OBJ_SWORD000;
extern const CfgObject CFG_OBJ_SKELETON000;
extern const CfgObject CFG_OBJ_PLAYER000;
extern const CfgObject CFG_OBJ_ARROW000;
extern const CfgObject CFG_OBJ_BULLET000;
extern const CfgObject CFG_OBJ_BULLET001;

typedef struct _LevelTile {
	const CfgGroundTile *gndTile;
	const CfgObject *obj;
} LevelTile;

typedef struct _CfgLevel {
	const LevelTile *tiles;
	int w, h;
} CfgLevel;

extern const CfgLevel CFG_LVL_SP000;

#endif
