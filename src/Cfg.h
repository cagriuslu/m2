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

typedef struct _CfgObjectTexture {
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
	const CfgObjectTexture* projectileTexture;
	float projectileSpeed; // m/s
	float projectileDamage;
	float rateOfFire;
	int projectileCount;
	float projectileTtl;
} CfgRangedWeapon;
extern const CfgRangedWeapon CFG_RANGEDWPN_GUN; // Default
extern const CfgRangedWeapon CFG_RANGEDWPN_MACHINEGUN; // Fast, thus powerful
extern const CfgRangedWeapon CFG_RANGEDWPN_SHOTGUN; // Slow but powerful
extern const CfgRangedWeapon CFG_RANGEDWPN_BOWNARRAY; // Slow, but piercing, thus default

typedef struct _CfgMeleeWeapon {
	const CfgObjectTexture* texture;
	float meleeDamage;
	float rateOfFire;
	enum _CfgMeleeWeaponMotion {
		MELEE_MOTION_SWING,
		MELEE_MOTION_STAB,
	} meleeMotion;
} CfgMeleeWeapon;
extern const CfgMeleeWeapon CFG_MELEEWPN_BAT; // Default
extern const CfgMeleeWeapon CFG_MELEEWPN_SWORD; // Slow but powerful
extern const CfgMeleeWeapon CFG_MELEEWPN_SPEAR; // Slow, High damage and pierce
extern const CfgMeleeWeapon CFG_MELEEWPN_DAGGER; // Fast, thus powerful

typedef struct _CfgExplosiveWeapon {
	const CfgObjectTexture* projectileTexture;
	float projectileSpeed; // m/s
	float projectileDamageMax;
	float projectileDamageMin;
	float projectileDamageRadius;
	float rateOfFire;
} CfgExplosiveWeapon;
extern const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADE;
extern const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADELAUNCHER;

typedef enum _CfgObjectType {
	CFG_OBJTYP_NONE = 0,
	CFG_OBJTYP_PLAYER,
	CFG_OBJTYP_ENEMY
} CfgObjectType;

typedef struct _CfgCharacter {
	const CfgObjectTexture* texture;
	CfgObjectType objType;
	float walkSpeed; // m/s
	float hp;
	const CfgRangedWeapon* defaultRangedWeapon;
	const CfgMeleeWeapon* defaultMeleeWeapon;
	const CfgExplosiveWeapon* defaultExplosiveWeapon;
	int defaultExplosiveCount;
} CfgCharacter;
extern const CfgCharacter CFG_CHARACTER_PLAYER;
extern const CfgCharacter CFG_CHARACTER_SKELETON000;

typedef struct _CfgLevel {
	const struct _LevelTile {
		const CfgGroundTexture* gndTile;
		const CfgObjectTexture* obj;
	} *tiles;
	int w, h;
} CfgLevel;
typedef struct _LevelTile LevelTile;
extern const CfgLevel CFG_LVL_SP000;

#endif
