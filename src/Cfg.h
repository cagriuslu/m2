#ifndef CFG_H
#define CFG_H

#include "Vec2F.h"
#include <SDL.h>

#define CFG_TILE_SIZE (24)
#define CFG_TEXTURE_FILE "resources/24.png"

typedef struct _CfgCollider {
	enum {
		CFG_COLLIDER_TYPE_NONE = 0,
		CFG_COLLIDER_TYPE_RECTANGLE,
		CFG_COLLIDER_TYPE_CIRCLE
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
	struct _CfgProjectile {
		const CfgObjectTexture *texture;
		float speed;
		float damage;
		float ttl;
	} projectile;
	unsigned projectileCount;
	float rateOfFire;
} CfgRangedWeapon;
typedef struct _CfgProjectile CfgProjectile;
extern const CfgRangedWeapon CFG_RANGEDWPN_GUN; // Default
extern const CfgRangedWeapon CFG_RANGEDWPN_MACHINEGUN; // Fast, thus powerful
extern const CfgRangedWeapon CFG_RANGEDWPN_SHOTGUN; // Slow but powerful
extern const CfgRangedWeapon CFG_RANGEDWPN_BOW; // Slow, but piercing, thus default
typedef struct _RangedWeaponState {
	float cooldownStopwatch;
} RangedWeaponState;

typedef struct _CfgMeleeWeapon {
	struct _CfgMelee {
		const CfgObjectTexture* texture;
		float damage;
		enum _CfgMeleeMotion {
			CFG_MELEE_MOTION_SWING,
			CFG_MELEE_MOTION_STAB,
		} motion;
	} melee;
	float rateOfFire;
} CfgMeleeWeapon;
typedef struct _CfgMelee CfgMelee;
typedef enum _CfgMeleeMotion CfgMeleeMotion;
extern const CfgMeleeWeapon CFG_MELEEWPN_BAT; // Default
extern const CfgMeleeWeapon CFG_MELEEWPN_SWORD; // Slow but powerful
extern const CfgMeleeWeapon CFG_MELEEWPN_SPEAR; // Slow, High damage and pierce
extern const CfgMeleeWeapon CFG_MELEEWPN_DAGGER; // Fast, thus powerful
typedef struct _MeleeWeaponState {
	float cooldownStopwatch;
} MeleeWeaponState;

typedef struct _CfgExplosiveWeapon {
	struct _CfgExplosive {
		const CfgObjectTexture* texture;
		float speed;
		float damageMax;
		float damageMin;
		float damageRadius;
	} explosive;
	float rateOfFire;
} CfgExplosiveWeapon;
typedef struct _CfgExplosive CfgExplosive;
extern const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADE;
extern const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADELAUNCHER;
typedef struct _ExplosiveWeaponState {
	float cooldownStopwatch;
} ExplosiveWeaponState;

typedef struct _CfgCharacter {
	const CfgObjectTexture* texture;
	enum _CfgObjectType {
		CFG_OBJTYP_NONE = 0,
		CFG_OBJTYP_PLAYER,
		CFG_OBJTYP_ENEMY
	} objType;
	float walkSpeed; // m/s
	float maxHp;
	const CfgRangedWeapon* defaultRangedWeapon;
	const CfgMeleeWeapon* defaultMeleeWeapon;
	const CfgExplosiveWeapon* defaultExplosiveWeapon;
	int defaultExplosiveCount;
} CfgCharacter;
typedef enum _CfgObjectType CfgObjectType;
extern const CfgCharacter CFG_CHARACTER_PLAYER;
extern const CfgCharacter CFG_CHARACTER_SKELETON000;
typedef struct CharacterState {
	float hp;
	RangedWeaponState rangedWeaponState;
	MeleeWeaponState meleeWeaponState;
	ExplosiveWeaponState explosiveWeaponState;
	unsigned explosiveCount;
} CharacterState;

typedef struct _CfgLevel {
	const struct _CfgLevelTile {
		const CfgGroundTexture* gndTile;
		const CfgCharacter *chr;
	} *tiles;
	int w, h;
} CfgLevel;
typedef struct _CfgLevelTile CfgLevelTile;
extern const CfgLevel CFG_LVL_SP000;

#endif
