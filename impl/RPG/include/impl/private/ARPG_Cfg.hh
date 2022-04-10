#ifndef IMPL_ARPG_CFG_HH
#define IMPL_ARPG_CFG_HH

#include <impl/private/ai/AiBlueprint.h>
#include <m2/SpriteBlueprint.h>
#include <m2/Def.h>
#include <list>

#define ARPG_CFG_TILE_SIZE (48)
#define ARPG_CFG_TILE_SIZE_F (48.0f)

////////////////////////////////////////////////////////////////////////
///////////////////// RANGED WEAPON AND PROJECTILE /////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct {
    m2::SpriteIndex spriteIndex;
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
    m2::SpriteIndex spriteIndex;
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
    m2::SpriteIndex spriteIndex;
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
    m2::SpriteIndex mainSpriteIndex;
	CfgObjectType objType;
	float mass_kg;
	float linearDamping;
	float walkSpeed;
	float maxHp;
	const CfgRangedWeapon* defaultRangedWeapon;
	const CfgMeleeWeapon* defaultMeleeWeapon;
	const CfgExplosiveWeapon* defaultExplosiveWeapon;
    m2::SpriteIndex spriteIndexes[CFG_CHARTEXTURETYP_N];
	const impl::ai::AiBlueprint* aiBlueprint;
} CfgCharacter;
extern const CfgCharacter CFG_CHARACTER_PLAYER;
extern const CfgCharacter CFG_CHARACTER_SKELETON_000_CHASE;
typedef struct CharacterState {
	const CfgCharacter *cfg;
	RangedWeaponState rangedWeaponState;
	MeleeWeaponState meleeWeaponState;
	ExplosiveWeaponState explosiveWeaponState;
} CharacterState;
M2Err CharacterState_Init(CharacterState* state, const CfgCharacter* cfg);
void CharacterState_ProcessTime(CharacterState* state, float timePassed);

#endif //IMPL_ARPG_CFG_HH
