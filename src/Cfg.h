#ifndef CFG_H
#define CFG_H

#include "Error.h"
#include "TinySet.h"
#include "Vec2F.h"
#include "Defs.h"
#include <SDL.h>

#define CFG_TILE_SIZE (24)
#define CFG_TEXTURE_FILE "resources/24.png"

typedef struct _CfgCollider {
	Vec2F center_px;
	Vec2F center_m;
	enum {
		CFG_COLLIDER_TYPE_INVALID = 0,
		CFG_COLLIDER_TYPE_RECTANGLE,
		CFG_COLLIDER_TYPE_CIRCLE
	} type;
	union {
		struct {
			Vec2F dims_px;
			Vec2F dims_m;
		} rect;
		struct {
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

typedef struct _CfgProjectile {
	const CfgObjectTexture* texture;
	float speed;
	float damage;
	float ttl;
} CfgProjectile;
typedef struct _CfgRangedWeapon {
	CfgProjectile projectile;
	unsigned projectileCount;
	float rateOfFire;
} CfgRangedWeapon;
extern const CfgRangedWeapon CFG_RANGEDWPN_GUN; // Default
extern const CfgRangedWeapon CFG_RANGEDWPN_MACHINEGUN; // Fast, thus powerful
extern const CfgRangedWeapon CFG_RANGEDWPN_SHOTGUN; // Slow but powerful
extern const CfgRangedWeapon CFG_RANGEDWPN_BOW; // Slow, but piercing, thus default
typedef struct _ProjectileState {
	const CfgProjectile* cfg;
	float ttl;
} ProjectileState;
typedef struct _RangedWeaponState {
	const CfgRangedWeapon *cfg;
	float cooldownStopwatch;
} RangedWeaponState;

typedef enum _CfgMeleeMotion {
	CFG_MELEE_MOTION_INVALID = 0,
	CFG_MELEE_MOTION_SWING,
	CFG_MELEE_MOTION_STAB,
} CfgMeleeMotion;
typedef struct _CfgMelee {
	const CfgObjectTexture* texture;
	float damage;
	CfgMeleeMotion motion;
	float ttl;
} CfgMelee;
typedef struct _CfgMeleeWeapon {
	CfgMelee melee;
	float rateOfFire;
} CfgMeleeWeapon;
typedef enum _CfgMeleeMotion CfgMeleeMotion;
extern const CfgMeleeWeapon CFG_MELEEWPN_BAT; // Default
extern const CfgMeleeWeapon CFG_MELEEWPN_SWORD; // Slow but powerful
extern const CfgMeleeWeapon CFG_MELEEWPN_SPEAR; // Slow, High damage and pierce
extern const CfgMeleeWeapon CFG_MELEEWPN_DAGGER; // Fast, thus powerful
typedef struct _MeleeState {
	const CfgMelee* cfg;
	float ttl;
} MeleeState;
typedef struct _MeleeWeaponState {
	const CfgMeleeWeapon *cfg;
	float cooldownStopwatch;
} MeleeWeaponState;

typedef struct _CfgExplosive {
	const CfgObjectTexture* texture;
	float projectileSpeed;
	float projectileTtl;
	float projectileRadius;
	float damageMax;
	float damageMin;
	float damageRadius;
} CfgExplosive;
typedef struct _CfgExplosiveWeapon {
	CfgExplosive explosive;
	float rateOfFire;
} CfgExplosiveWeapon;
extern const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADE;
extern const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADELAUNCHER;
typedef enum _ExplosiveStatus {
	EXPLOSIVE_STATUS_INVALID = 0,
	EXPLOSIVE_STATUS_IN_FLIGHT,
	EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP,
	EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP
} ExplosiveStatus;
typedef struct _ExplosiveState {
	const CfgExplosive* cfg;
	float projectileTtl;
	ExplosiveStatus explosiveStatus;
	TinySetOfIDs damagedObjs;
} ExplosiveState;
typedef struct _ExplosiveWeaponState {
	const CfgExplosiveWeapon *cfg;
	float cooldownStopwatch;
} ExplosiveWeaponState;

typedef enum _CfgObjectType {
	CFG_OBJTYP_INVALID = 0,
	CFG_OBJTYP_PLAYER,
	CFG_OBJTYP_ENEMY
} CfgObjectType;
typedef struct _CfgCharacter {
	const CfgObjectTexture* texture;
	CfgObjectType objType;
	float walkSpeed; // m/s
	float maxHp;
	const CfgRangedWeapon* defaultRangedWeapon;
	const CfgMeleeWeapon* defaultMeleeWeapon;
	const CfgExplosiveWeapon* defaultExplosiveWeapon;
	int defaultExplosiveCount;
} CfgCharacter;
extern const CfgCharacter CFG_CHARACTER_PLAYER;
extern const CfgCharacter CFG_CHARACTER_SKELETON000;
typedef struct CharacterState {
	const CfgCharacter *cfg;
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

typedef enum _CfgMarkupButtonType {
	CFG_MARKUP_BUTTON_TYPE_INVALID = 0,
	CFG_MARKUP_BUTTON_TYPE_NEW_GAME,
	CFG_MARKUP_BUTTON_TYPE_QUIT,
	CFG_MARKUP_BUTTON_TYPE_RESUME,
} CfgMarkupButtonType;
typedef enum _CfgMarkupDynamicTextType {
	CFG_MARKUP_DYNAMIC_TEXT_TYPE_INVALID = 0,
	CFG_MARKUP_DYNAMIC_TEXT_TYPE_HP
} CfgMarkupDynamicTextType;
typedef enum _CfgMarkupDynamicImageType {
	CFG_MARKUP_DYNAMIC_IMAGE_TYPE_INVALID = 0,
} CfgMarkupDynamicImageType;
typedef enum _CfgMarkupElementType {
	CFG_MARKUP_ELEMENT_TYPE_INVALID = 0,
	CFG_MARKUP_ELEMENT_TYPE_MARKUP,
	CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT,
	CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON,
	CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE,
	CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE_BUTTON,
	CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT,
	CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON,
	CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE,
	CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON,
} CfgMarkupElementType;
struct _CfgMarkup;
typedef struct _CfgMarkupElement {
	unsigned x, y, w, h; // unitless
	unsigned borderWidth_px;
	SDL_Color backgroundColor;
	
	CfgMarkupElementType type;
	// Exists for MARKUP
	const struct _CfgMarkup* child;
	// Exists for STATIC_TEXT, STATIC_TEXT_BUTTON
	const char* text;
	// Exists for STATIC_TEXT_BUTTON, STATIC_IMAGE_BUTTON, DYNAMIC_TEXT_BUTTON, DYNAMIC_IMAGE_BUTTON
	CfgMarkupButtonType buttonType;
	SDL_Scancode keyboardShortcut;
	// Exists for STATIC_IMAGE, STATIC_IMAGE_BUTTON
	const CfgObjectTexture* texture;
	// Exists for DYNAMIC_TEXT, DYNAMIC_TEXT_BUTTON
	CfgMarkupDynamicTextType textType;
	// Exists for DYNAMIC_IMAGE, DYNAMIC_IMAGE_BUTTON
	CfgMarkupDynamicImageType imageType;
	
	const struct _CfgMarkupElement* next;
} CfgMarkupElement;
typedef struct _CfgMarkup {
	unsigned w, h; // unitless
	unsigned borderWidth_px;
	SDL_Color backgroundColor;
	const CfgMarkupElement* firstElement;
} CfgMarkup;
DECLARE_SIBLING_LIST_LENGTH_CALCULATOR(CfgMarkupElement);
extern const CfgMarkup CFG_MARKUP_START_MENU;
extern const CfgMarkup CFG_MARKUP_PAUSE_MENU;
extern const CfgMarkup CFG_MARKUP_HUD_LEFT;
typedef struct _MarkupElementState {
	SDL_Rect rect;
	const CfgMarkupElement* cfg;

	// Exists for MARKUP
	struct _MarkupState* child;
	// Exists for STATIC_TEXT, STATIC_TEXT_BUTTON, DYNAMIC_TEXT, DYNAMIC_TEXT_BUTTON
	SDL_Texture* textTexture;
	// Exists for STATIC_TEXT_BUTTON, STATIC_IMAGE_BUTTON, DYNAMIC_TEXT_BUTTON, DYNAMIC_IMAGE_BUTTON
	bool depressed;
	// Exists for DYNAMIC_IMAGE, DYNAMIC_IMAGE_BUTTON
	const CfgObjectTexture* texture;
	
	struct _MarkupElementState* next;
} MarkupElementState;
typedef struct _MarkupState {
	const CfgMarkup *cfg;
	SDL_Rect rect;
	MarkupElementState* firstElement;
} MarkupState;

#endif
