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

typedef struct _CfgGroundTexture {
	SDL_Rect textureRect;
	CfgCollider collider;
} CfgGroundTexture;
extern const CfgGroundTexture CFG_GNDTXTR_DEFAULT;
extern const CfgGroundTexture CFG_GNDTXTR_GROUND000;
extern const CfgGroundTexture CFG_GNDTXTR_CLIFF000T;
extern const CfgGroundTexture CFG_GNDTXTR_CLIFF000R;
extern const CfgGroundTexture CFG_GNDTXTR_CLIFF000B;
extern const CfgGroundTexture CFG_GNDTXTR_CLIFF000L;
extern const CfgGroundTexture CFG_GNDTXTR_CLIFF000TR;
extern const CfgGroundTexture CFG_GNDTXTR_CLIFF000TL;
extern const CfgGroundTexture CFG_GNDTXTR_CLIFF000BR;
extern const CfgGroundTexture CFG_GNDTXTR_CLIFF000BL;

#define CFG_OBJ_ID_PLAYER (1)
#define CFG_OBJ_ID_ENEMY  (2)
typedef struct _CfgObjectTexture {
	int id;
	SDL_Rect textureRect;
	Vec2F objCenter_px;
	Vec2F objCenter_m;
	CfgCollider collider;
} CfgObjectTexture;
extern const CfgObjectTexture CFG_OBJTXTR_BOX000;
extern const CfgObjectTexture CFG_OBJTXTR_SWORD000;
extern const CfgObjectTexture CFG_OBJTXTR_SKELETON000;
extern const CfgObjectTexture CFG_OBJTXTR_PLAYER000;
extern const CfgObjectTexture CFG_OBJTXTR_ARROW000;
extern const CfgObjectTexture CFG_OBJTXTR_BULLET000;
extern const CfgObjectTexture CFG_OBJTXTR_BULLET001;

typedef struct _CfgRangedWeapon {
	CfgObjectTexture* projectileTexture;
	float projectileSpeed; // m/s
	float projectileDamage;
	int projectileCount;
	float rateOfFire;
} CfgRangedWeapon;
extern const CfgRangedWeapon CFG_RANGEDWPN_GUN;
extern const CfgRangedWeapon CFG_RANGEDWPN_MACHINEGUN;
extern const CfgRangedWeapon CFG_RANGEDWPN_SHOTGUN;
extern const CfgRangedWeapon CFG_RANGEDWPN_BOWNARRAY;

typedef struct _CfgMeleeWeapon {
	CfgObjectTexture* texture;
	float meleeDamage;
	float rateOfFire;
} CfgMeleeWeapon;
extern const CfgMeleeWeapon CFG_RANGEDWPN_BAT; // Default
extern const CfgMeleeWeapon CFG_RANGEDWPN_SWORD; // Slow but powerful
extern const CfgMeleeWeapon CFG_RANGEDWPN_SPEAR; // High damage and pierce
extern const CfgMeleeWeapon CFG_RANGEDWPN_DAGGER; // Fast, thus powerful

typedef struct _CfgExplosiveWeapon {
	CfgObjectTexture* projectileTexture;
	float projectileSpeed; // m/s
	float projectileDamage;
	float projectileDamageRadius;
	float rateOfFire;
} CfgExplosiveWeapon;
extern const CfgRangedWeapon CFG_RANGEDWPN_GRENADE;
extern const CfgRangedWeapon CFG_RANGEDWPN_GRENADELAUNCHER;

typedef struct _CfgCharacter {
	CfgObjectTexture* texture;
	
} CfgCharacter;

typedef struct _CfgPlayer {
	CfgCharacter character;
} CfgPlayer;

// Character
// Weapon
// Projectile
// Explosive

typedef struct _LevelTile {
	const CfgGroundTexture *gndTile;
	const CfgObjectTexture *obj;
} LevelTile;

typedef struct _CfgLevel {
	const LevelTile *tiles;
	int w, h;
} CfgLevel;

extern const CfgLevel CFG_LVL_SP000;

#endif
