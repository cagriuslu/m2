#ifndef CFG_H
#define CFG_H

#include "TinySet.h"
#include "Automaton.h"
#include "List.h"
#include "Vec2F.h"
#include "Def.h"
#include <SDL.h>

#define CFG_TILE_SIZE (48)
#define CFG_TILE_SIZE_F (48.0f)
#define CFG_TEXTURE_FILE "resources/48.png"
#define CFG_TEXTURE_MASK_FILE "resources/48-Mask.png"

////////////////////////////////////////////////////////////////////////
//////////////////////////// GROUND TEXTURE ////////////////////////////
////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////
//////////////////////////// OBJECT TEXTURE ////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct _CfgObjectTexture {
	SDL_Rect textureRect;
	Vec2F objCenter_px;
	Vec2F objCenter_m;
	CfgCollider collider;
} CfgObjectTexture;
extern const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKDOWN_00;
extern const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKDOWN_01;
extern const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKDOWN_02;
extern const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKLEFT_00;
extern const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKLEFT_01;
extern const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKLEFT_02;
extern const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKRIGHT_00;
extern const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKRIGHT_01;
extern const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKRIGHT_02;
extern const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKUP_00;
extern const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKUP_01;
extern const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKUP_02;
extern const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKDOWN_00;
extern const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKDOWN_01;
extern const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKDOWN_02;
extern const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKLEFT_00;
extern const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKLEFT_01;
extern const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKLEFT_02;
extern const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKRIGHT_00;
extern const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKRIGHT_01;
extern const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKRIGHT_02;
extern const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKUP_00;
extern const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKUP_01;
extern const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKUP_02;
extern const CfgObjectTexture CFG_OBJTXTR_BULLET_00;
extern const CfgObjectTexture CFG_OBJTXTR_BULLET_01;
extern const CfgObjectTexture CFG_OBJTXTR_SWORD_00;
extern const CfgObjectTexture CFG_OBJTXTR_BOMB_00;

////////////////////////////////////////////////////////////////////////
///////////////////// RANGED WEAPON AND PROJECTILE /////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct _CfgProjectile {
	const CfgObjectTexture* texture;
	float speed_mps;
	float damage;
	float ttl_s;
	float damageAccuracy;
	float ttlAccuracy;
} CfgProjectile;
typedef struct _ProjectileState {
	const CfgProjectile* cfg;
	bool alreadyCollidedThisStep;
	float ttl_s;
} ProjectileState;
XErr ProjectileState_Init(ProjectileState* state, const CfgProjectile* cfg);

typedef struct _CfgRangedWeapon {
	CfgProjectile projectile;
	unsigned projectileCount;
	float cooldown_s;
	float accuracy;
} CfgRangedWeapon;
extern const CfgRangedWeapon CFG_RANGEDWPN_GUN; // Default
extern const CfgRangedWeapon CFG_RANGEDWPN_MACHINEGUN; // Fast, thus powerful
extern const CfgRangedWeapon CFG_RANGEDWPN_SHOTGUN; // Slow but powerful
extern const CfgRangedWeapon CFG_RANGEDWPN_BOW; // Slow, but piercing, thus default
typedef struct _RangedWeaponState {
	const CfgRangedWeapon *cfg;
	float cooldownCounter_s;
} RangedWeaponState;
XErr RangedWeaponState_Init(RangedWeaponState* state, const CfgRangedWeapon* cfg);
void RangedWeaponState_ProcessTime(RangedWeaponState* state, float timePassed);

////////////////////////////////////////////////////////////////////////
//////////////////////// MELEE WEAPON AND MELEE ////////////////////////
////////////////////////////////////////////////////////////////////////
typedef enum _CfgMeleeMotion {
	CFG_MELEE_MOTION_INVALID = 0,
	CFG_MELEE_MOTION_SWING,
	CFG_MELEE_MOTION_STAB,
} CfgMeleeMotion;
typedef struct _CfgMelee {
	const CfgObjectTexture* texture;
	float damage;
	CfgMeleeMotion motion;
	float ttl_s;
} CfgMelee;
typedef struct _MeleeState {
	const CfgMelee* cfg;
	float ttl_s;
} MeleeState;
XErr MeleeState_Init(MeleeState* state, const CfgMelee* cfg);

typedef struct _CfgMeleeWeapon {
	CfgMelee melee;
	float cooldown_s;
} CfgMeleeWeapon;
typedef enum _CfgMeleeMotion CfgMeleeMotion;
extern const CfgMeleeWeapon CFG_MELEEWPN_BAT; // Default
extern const CfgMeleeWeapon CFG_MELEEWPN_SWORD; // Slow but powerful
extern const CfgMeleeWeapon CFG_MELEEWPN_SPEAR; // Slow, High damage and pierce
extern const CfgMeleeWeapon CFG_MELEEWPN_DAGGER; // Fast, thus powerful
typedef struct _MeleeWeaponState {
	const CfgMeleeWeapon *cfg;
	float cooldownCounter_s;
} MeleeWeaponState;
XErr MeleeWeaponState_Init(MeleeWeaponState* state, const CfgMeleeWeapon* cfg);
void MeleeWeaponState_ProcessTime(MeleeWeaponState* state, float timePassed);

////////////////////////////////////////////////////////////////////////
//////////////////// EXPLOSIVE WEAPON AND EXPLOSIVE ////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct _CfgExplosive {
	const CfgObjectTexture* texture;
	float projectileSpeed_mps;
	float projectileTtl_s;
	float projectileBodyRadius_m;
	float damageMax;
	float damageMin;
	float damageRadius_m;
} CfgExplosive;
typedef enum _ExplosiveStatus {
	EXPLOSIVE_STATUS_INVALID = 0,
	EXPLOSIVE_STATUS_IN_FLIGHT,
	EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP,
	EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP
} ExplosiveStatus;
typedef struct _ExplosiveState {
	const CfgExplosive* cfg;
	float projectileTtl_s;
	ExplosiveStatus explosiveStatus;
} ExplosiveState;
XErr ExplosiveState_Init(ExplosiveState* state, const CfgExplosive* cfg);

typedef struct _CfgExplosiveWeapon {
	CfgExplosive explosive;
	float cooldown_s;
	unsigned initialExplosiveCount;
} CfgExplosiveWeapon;
extern const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADE;
extern const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADELAUNCHER;
typedef struct _ExplosiveWeaponState {
	const CfgExplosiveWeapon *cfg;
	float cooldownCounter_s;
	unsigned explosiveCount;
} ExplosiveWeaponState;
XErr ExplosiveWeaponState_Init(ExplosiveWeaponState* state, const CfgExplosiveWeapon* cfg);
void ExplosiveWeaponState_ProcessTime(ExplosiveWeaponState* state, float timePassed);

////////////////////////////////////////////////////////////////////////
////////////////////////////////// AI //////////////////////////////////
////////////////////////////////////////////////////////////////////////
// Behavior corresponds to the state machine that is used
typedef enum _CfgAiBehavior {
	CFG_AI_BEHAVIOR_INVALID = 0,
	CFG_AI_BEHAVIOR_CHASE,
	CFG_AI_BEHAVIOR_KEEP_DISTANCE,
	CFG_AI_BEHAVIOR_HIT_N_RUN,
	CFG_AI_BEHAVIOR_PATROL, // Chases the player when triggered
	CFG_AI_BEHAVIOR_CIRCLE_AROUND
} CfgAiBehavior;
typedef enum _CfgAiCapability {
	CFG_AI_CAPABILITY_INVALID = 0,
	CFG_AI_CAPABILITY_RANGED,
	CFG_AI_CAPABILITY_MELEE,
	CFG_AI_CAPABILITY_EXPLOSIVE,
	CFG_AI_CAPABILITY_KAMIKAZE
} CfgAiCapability;
typedef struct _CfgAiDescriptor {
	CfgAiBehavior behavior;
	CfgAiCapability capability;
	/// Distance AI becomes active
	float triggerDistanceSquared_m;
	/// Distance AI is clear to attack player
	float attackDistanceSquared_m;
	/// Distance AI gives up and returns home
	float giveUpDistanceSquared_m;
	/// Period after which AI recalculates waypoints (formula: random(s/2, 3s/2))
	float recalculationPeriod_s;
	/// If behavior=KEEP_DISTANCE, distance AI tries to keep
	float keepDistanceDistanceSquared_m;
	/// If behavior=HIT_N_RUN, distance AI tries to achieve during Hit period
	float hitNRunHitDistanceSquared_m;
	/// If behavior=HIT_N_RUN, duration AI stays in Hit period
	float hitNRunHitDuration_s;
	/// If behavior=HIT_N_RUN, distance AI tries to achieve during Run period
	float hitNRunRunDistanceSquared_m;
	/// If behavior=HIT_N_RUN, duration AI stays in Run period
	float hitNRunRunDuration_s;
	/// If behavior=PATROL, top-left of patrol area while idling
	Vec2F patrolAreaTopLeft;
	/// If behavior=PATROL, bottom-right of patrol area while idling
	Vec2F patrolAreaBottomRight;
	/// If behavior=PATROL, patrol speed
	float patrolSpeed_mps;
	// Circle around parameters
	// TODO
} CfgAi;
extern const CfgAi CFG_AI_CHASE_00;
extern const CfgAi CFG_AI_KEEP_DISTANCE_00;
extern const CfgAi CFG_AI_HIT_N_RUN_00;
typedef struct _AiState {
	const CfgAi* cfg;
	Vec2F homePosition;
	ListOfVec2Is reversedWaypointList;
} AiState;
XErr AiState_Init(AiState *state, const CfgAi* cfg, Vec2F homePosition);

////////////////////////////////////////////////////////////////////////
/////////////////////////////// CHARACTER //////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef enum _CfgObjectType {
	CFG_OBJTYP_INVALID = 0,
	CFG_OBJTYP_PLAYER,
	CFG_OBJTYP_ENEMY
} CfgObjectType;
typedef enum _CfgCharacterTextureType {
	CFG_CHARTEXTURETYP_LOOKDOWN_00 = 0,
	CFG_CHARTEXTURETYP_LOOKDOWN_01,
	CFG_CHARTEXTURETYP_LOOKDOWN_02,
	CFG_CHARTEXTURETYP_LOOKLEFT_00,
	CFG_CHARTEXTURETYP_LOOKLEFT_01,
	CFG_CHARTEXTURETYP_LOOKLEFT_02,
	CFG_CHARTEXTURETYP_LOOKRIGHT_00,
	CFG_CHARTEXTURETYP_LOOKRIGHT_01,
	CFG_CHARTEXTURETYP_LOOKRIGHT_02,
	CFG_CHARTEXTURETYP_LOOKUP_00,
	CFG_CHARTEXTURETYP_LOOKUP_01,
	CFG_CHARTEXTURETYP_LOOKUP_02,
	CFG_CHARTEXTURETYP_N
} CfgCharacterTextureType;
typedef struct _CfgCharacter {
	const CfgObjectTexture* mainTexture;
	CfgObjectType objType;
	float mass_kg;
	float walkSpeed_mps;
	float maxHp;
	const CfgRangedWeapon* defaultRangedWeapon;
	const CfgMeleeWeapon* defaultMeleeWeapon;
	const CfgExplosiveWeapon* defaultExplosiveWeapon;
	const CfgObjectTexture* textures[CFG_CHARTEXTURETYP_N];
	const CfgAi* ai;
} CfgCharacter;
extern const CfgCharacter CFG_CHARACTER_PLAYER;
extern const CfgCharacter CFG_CHARACTER_SKELETON_000_CHASE;
extern const CfgCharacter CFG_CHARACTER_SKELETON_000_KEEP_DISTANCE;
extern const CfgCharacter CFG_CHARACTER_SKELETON_000_HIT_N_RUN;
typedef struct CharacterState {
	const CfgCharacter *cfg;
	RangedWeaponState rangedWeaponState;
	MeleeWeaponState meleeWeaponState;
	ExplosiveWeaponState explosiveWeaponState;
} CharacterState;
XErr CharacterState_Init(CharacterState* state, const CfgCharacter* cfg);
void CharacterState_ProcessTime(CharacterState* state, float timePassed);

////////////////////////////////////////////////////////////////////////
///////////////////////////////// LEVEL ////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct _CfgLevel {
	const struct _CfgLevelTile {
		const CfgGroundTexture* gndTile;
		const CfgCharacter *chr;
	} *tiles;
	int w, h;
} CfgLevel;
typedef struct _CfgLevelTile CfgLevelTile;
extern const CfgLevel CFG_LVL_SP000;

////////////////////////////////////////////////////////////////////////
//////////////////////////////// MARKUP ////////////////////////////////
////////////////////////////////////////////////////////////////////////
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
extern const CfgMarkup CFG_MARKUP_HUD_RIGHT;
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
