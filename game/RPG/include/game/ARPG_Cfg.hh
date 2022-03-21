#ifndef M2_ARPG_CFG_HH
#define M2_ARPG_CFG_HH

#include "m2/Cfg.hh"
#include <list>

#define ARPG_CFG_TILE_SIZE (48)
#define ARPG_CFG_TILE_SIZE_F (48.0f)

////////////////////////////////////////////////////////////////////////
////////////////////////////////// UI //////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef enum {
	ARPG_CFG_UI_BUTTONTYPE_NEWGAME = CFG_UI_BUTTONTYP_N,
	ARPG_CFG_UI_BUTTONTYPE_RESUME
} ARPG_CfgUIButtonType;

extern const CfgUI ARPG_CFG_UI_ENTRYUI;
extern const CfgUI ARPG_CFG_UI_PAUSEUI;

extern const CfgUI CFG_UI_HUDLEFT;
extern const CfgUI CFG_UI_HUDRIGHT;

////////////////////////////////////////////////////////////////////////
/////////////////////////////// SPRITES ////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef enum {
	ARPG_CFGSPRITE_NONE = 0,
	ARPG_CFGSPRITE_DEFAULT,
	ARPG_CFGSPRITE_GRASS,
	ARPG_CFGSPRITE_CLIFF000T,
	ARPG_CFGSPRITE_CLIFF000R,
	ARPG_CFGSPRITE_CLIFF000B,
	ARPG_CFGSPRITE_CLIFF000L,
	ARPG_CFGSPRITE_CLIFF000TR,
	ARPG_CFGSPRITE_CLIFF000TL,
	ARPG_CFGSPRITE_CLIFF000BR,
	ARPG_CFGSPRITE_CLIFF000BL,

	ARPG_CFGSPRITE_PLAYER_LOOKDOWN_00,
	ARPG_CFGSPRITE_PLAYER_LOOKDOWN_01,
	ARPG_CFGSPRITE_PLAYER_LOOKDOWN_02,
	ARPG_CFGSPRITE_PLAYER_LOOKLEFT_00,
	ARPG_CFGSPRITE_PLAYER_LOOKLEFT_01,
	ARPG_CFGSPRITE_PLAYER_LOOKLEFT_02,
	ARPG_CFGSPRITE_PLAYER_LOOKRIGHT_00,
	ARPG_CFGSPRITE_PLAYER_LOOKRIGHT_01,
	ARPG_CFGSPRITE_PLAYER_LOOKRIGHT_02,
	ARPG_CFGSPRITE_PLAYER_LOOKUP_00,
	ARPG_CFGSPRITE_PLAYER_LOOKUP_01,
	ARPG_CFGSPRITE_PLAYER_LOOKUP_02,
	ARPG_CFGSPRITE_ENEMY_LOOKDOWN_00,
	ARPG_CFGSPRITE_ENEMY_LOOKDOWN_01,
	ARPG_CFGSPRITE_ENEMY_LOOKDOWN_02,
	ARPG_CFGSPRITE_ENEMY_LOOKLEFT_00,
	ARPG_CFGSPRITE_ENEMY_LOOKLEFT_01,
	ARPG_CFGSPRITE_ENEMY_LOOKLEFT_02,
	ARPG_CFGSPRITE_ENEMY_LOOKRIGHT_00,
	ARPG_CFGSPRITE_ENEMY_LOOKRIGHT_01,
	ARPG_CFGSPRITE_ENEMY_LOOKRIGHT_02,
	ARPG_CFGSPRITE_ENEMY_LOOKUP_00,
	ARPG_CFGSPRITE_ENEMY_LOOKUP_01,
	ARPG_CFGSPRITE_ENEMY_LOOKUP_02,
	ARPG_CFGSPRITE_BULLET_00,
	ARPG_CFGSPRITE_BULLET_01,
	ARPG_CFGSPRITE_SWORD_00,
	ARPG_CFGSPRITE_BOMB_00,

	ARPG_CFGSPRITE_N
} ARPG_CfgSprite_Index;

extern const CfgSprite ARPG_CFG_SPRITES[];

////////////////////////////////////////////////////////////////////////
///////////////////////////////// LEVEL ////////////////////////////////
////////////////////////////////////////////////////////////////////////
extern const CfgLevel CFG_LVL_SP000;

////////////////////////////////////////////////////////////////////////
///////////////////// RANGED WEAPON AND PROJECTILE /////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct {
	CfgSpriteIndex spriteIndex;
	float speed_mps;
	float damage;
	float ttl_s;
	float damageAccuracy;
	float ttlAccuracy;
} CfgProjectile;
typedef struct {
	const CfgProjectile* cfg;
	bool alreadyCollidedThisStep;
	float ttl_s;
} ProjectileState;
M2Err ProjectileState_Init(ProjectileState* state, const CfgProjectile* cfg);

typedef struct {
	CfgProjectile projectile;
	unsigned projectileCount;
	float cooldown_s;
	float accuracy;
} CfgRangedWeapon;
extern const CfgRangedWeapon CFG_RANGEDWPN_GUN; // Default
extern const CfgRangedWeapon CFG_RANGEDWPN_MACHINEGUN; // Fast, thus powerful
extern const CfgRangedWeapon CFG_RANGEDWPN_SHOTGUN; // Slow but powerful
extern const CfgRangedWeapon CFG_RANGEDWPN_BOW; // Slow, but piercing, thus default
typedef struct {
	const CfgRangedWeapon *cfg;
	float cooldownCounter_s;
} RangedWeaponState;
M2Err RangedWeaponState_Init(RangedWeaponState* state, const CfgRangedWeapon* cfg);
void RangedWeaponState_ProcessTime(RangedWeaponState* state, float timePassed);

////////////////////////////////////////////////////////////////////////
//////////////////////// MELEE WEAPON AND MELEE ////////////////////////
////////////////////////////////////////////////////////////////////////
typedef enum {
	CFG_MELEE_MOTION_INVALID = 0,
	CFG_MELEE_MOTION_SWING,
	CFG_MELEE_MOTION_STAB,
} CfgMeleeMotion;
typedef struct {
	CfgSpriteIndex spriteIndex;
	float damage;
	CfgMeleeMotion motion;
	float ttl_s;
} CfgMelee;
typedef struct {
	const CfgMelee* cfg;
	float ttl_s;
} MeleeState;
M2Err MeleeState_Init(MeleeState* state, const CfgMelee* cfg);

typedef struct {
	CfgMelee melee;
	float cooldown_s;
} CfgMeleeWeapon;
extern const CfgMeleeWeapon CFG_MELEEWPN_BAT; // Default
extern const CfgMeleeWeapon CFG_MELEEWPN_SWORD; // Slow but powerful
extern const CfgMeleeWeapon CFG_MELEEWPN_SPEAR; // Slow, High damage and pierce
extern const CfgMeleeWeapon CFG_MELEEWPN_DAGGER; // Fast, thus powerful
typedef struct {
	const CfgMeleeWeapon *cfg;
	float cooldownCounter_s;
} MeleeWeaponState;
M2Err MeleeWeaponState_Init(MeleeWeaponState* state, const CfgMeleeWeapon* cfg);
void MeleeWeaponState_ProcessTime(MeleeWeaponState* state, float timePassed);

////////////////////////////////////////////////////////////////////////
//////////////////// EXPLOSIVE WEAPON AND EXPLOSIVE ////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct {
	CfgSpriteIndex spriteIndex;
	float projectileSpeed_mps;
	float projectileTtl_s;
	float projectileBodyRadius_m;
	float damageMax;
	float damageMin;
	float damageRadius_m;
} CfgExplosive;
typedef enum {
	EXPLOSIVE_STATUS_INVALID = 0,
	EXPLOSIVE_STATUS_IN_FLIGHT,
	EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP,
	EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP
} ExplosiveStatus;
typedef struct {
	const CfgExplosive* cfg;
	float projectileTtl_s;
	ExplosiveStatus explosiveStatus;
} ExplosiveState;
M2Err ExplosiveState_Init(ExplosiveState* state, const CfgExplosive* cfg);

typedef struct {
	CfgExplosive explosive;
	float cooldown_s;
	unsigned initialExplosiveCount;
} CfgExplosiveWeapon;
extern const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADE;
extern const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADELAUNCHER;
typedef struct {
	const CfgExplosiveWeapon *cfg;
	float cooldownCounter_s;
	unsigned explosiveCount;
} ExplosiveWeaponState;
M2Err ExplosiveWeaponState_Init(ExplosiveWeaponState* state, const CfgExplosiveWeapon* cfg);
void ExplosiveWeaponState_ProcessTime(ExplosiveWeaponState* state, float timePassed);

////////////////////////////////////////////////////////////////////////
////////////////////////////////// AI //////////////////////////////////
////////////////////////////////////////////////////////////////////////
// Behavior corresponds to the state machine that is used
typedef enum {
	CFG_AI_BEHAVIOR_INVALID = 0,
	CFG_AI_BEHAVIOR_CHASE,
	CFG_AI_BEHAVIOR_KEEP_DISTANCE,
	CFG_AI_BEHAVIOR_HIT_N_RUN,
	CFG_AI_BEHAVIOR_PATROL, // Chases the player when triggered
	CFG_AI_BEHAVIOR_CIRCLE_AROUND
} CfgAiBehavior;
typedef enum {
	CFG_AI_CAPABILITY_INVALID = 0,
	CFG_AI_CAPABILITY_RANGED,
	CFG_AI_CAPABILITY_MELEE,
	CFG_AI_CAPABILITY_EXPLOSIVE,
	CFG_AI_CAPABILITY_KAMIKAZE
} CfgAiCapability;
typedef struct {
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
	m2::vec2f patrolAreaTopLeft;
	/// If behavior=PATROL, bottom-right of patrol area while idling
	m2::vec2f patrolAreaBottomRight;
	/// If behavior=PATROL, patrol speed
	float patrolSpeed_mps;
	// Circle around parameters
	// TODO
} CfgAi;
extern const CfgAi CFG_AI_CHASE_00;
extern const CfgAi CFG_AI_KEEP_DISTANCE_00;
extern const CfgAi CFG_AI_HIT_N_RUN_00;
struct AiState {
	const CfgAi* cfg;
	m2::vec2f homePosition;
    std::list<m2::vec2i> reversedWaypointList;
};
M2Err AiState_Init(AiState *state, const CfgAi* cfg, m2::vec2f homePosition);

////////////////////////////////////////////////////////////////////////
/////////////////////////////// CHARACTER //////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef enum {
	CFG_OBJTYP_INVALID = 0,
	CFG_OBJTYP_PLAYER,
	CFG_OBJTYP_ENEMY
} CfgObjectType;
typedef enum {
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
typedef struct {
	CfgSpriteIndex mainSpriteIndex;
	CfgObjectType objType;
	float mass_kg;
	float linearDamping;
	float walkSpeed;
	float maxHp;
	const CfgRangedWeapon* defaultRangedWeapon;
	const CfgMeleeWeapon* defaultMeleeWeapon;
	const CfgExplosiveWeapon* defaultExplosiveWeapon;
	CfgSpriteIndex spriteIndexes[CFG_CHARTEXTURETYP_N];
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
M2Err CharacterState_Init(CharacterState* state, const CfgCharacter* cfg);
void CharacterState_ProcessTime(CharacterState* state, float timePassed);

#endif //M2_ARPG_CFG_HH
