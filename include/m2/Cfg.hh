#ifndef CFG_H
#define CFG_H

#include "m2/FSM.h"
#include <m2/Vec2f.h>
#include "Def.hh"
#include <SDL.h>

////////////////////////////////////////////////////////////////////////
/////////////////////////////// SPRITES ////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef enum {
	CFG_COLLIDER_TYPE_INVALID = 0,
	CFG_COLLIDER_TYPE_RECTANGLE,
	CFG_COLLIDER_TYPE_CIRCLE
} CfgColliderType;
typedef struct {
	m2::Vec2f center_px;
	m2::Vec2f center_m;
	CfgColliderType type;
	union {
		struct {
			m2::Vec2f dims_px;
			m2::Vec2f dims_m;
		} rect;
		struct {
			float radius_px;
			float radius_m;
		} circ;
	} params;
} CfgCollider;

typedef uint32_t CfgSpriteIndex;

typedef struct {
	CfgSpriteIndex index;
	SDL_Rect textureRect;
	m2::Vec2f objCenter_px;
	m2::Vec2f objCenter_m;
	CfgCollider collider;
} CfgSprite;

typedef struct {
	CfgSpriteIndex backgroundSpriteIndex;
	CfgSpriteIndex foregroundSpriteIndex;
} CfgTile;

////////////////////////////////////////////////////////////////////////
//////////////////////////////// LEVEL /////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct {
	const CfgTile *tiles;
	int w, h;
} CfgLevel;

#endif
