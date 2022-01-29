#include "Cfg.h"

////////////////////////////////////////////////////////////////////////
///////////////////// RANGED WEAPON AND PROJECTILE /////////////////////
////////////////////////////////////////////////////////////////////////

XErr ProjectileState_Init(ProjectileState* state, const CfgProjectile* cfg) {
	memset(state, 0, sizeof(ProjectileState));
	state->cfg = cfg;
	state->ttl_s = cfg->ttl_s;
	return XOK;
}

XErr RangedWeaponState_Init(RangedWeaponState* state, const CfgRangedWeapon* cfg) {
	memset(state, 0, sizeof(RangedWeaponState));
	state->cfg = cfg;
	state->cooldownCounter_s = cfg->cooldown_s;
	return XOK;
}

////////////////////////////////////////////////////////////////////////
//////////////////////// MELEE WEAPON AND MELEE ////////////////////////
////////////////////////////////////////////////////////////////////////

XErr MeleeState_Init(MeleeState* state, const CfgMelee* cfg) {
	memset(state, 0, sizeof(MeleeState));
	state->cfg = cfg;
	state->ttl_s = cfg->ttl_s;
	return XOK;
}

XErr MeleeWeaponState_Init(MeleeWeaponState* state, const CfgMeleeWeapon* cfg) {
	memset(state, 0, sizeof(MeleeWeaponState));
	state->cfg = cfg;
	return XOK;
}

////////////////////////////////////////////////////////////////////////
//////////////////// EXPLOSIVE WEAPON AND EXPLOSIVE ////////////////////
////////////////////////////////////////////////////////////////////////

XErr ExplosiveState_Init(ExplosiveState* state, const CfgExplosive* cfg) {
	memset(state, 0, sizeof(ExplosiveState));
	state->cfg = cfg;
	state->projectileTtl_s = cfg->projectileTtl_s;
	return XOK;
}

XErr ExplosiveWeaponState_Init(ExplosiveWeaponState* state, const CfgExplosiveWeapon* cfg) {
	memset(state, 0, sizeof(ExplosiveWeaponState));
	state->cfg = cfg;
	return XOK;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////// AI //////////////////////////////////
////////////////////////////////////////////////////////////////////////

XErr AiState_Init(AiState *state, const CfgAi* cfg) {
	memset(state, 0, sizeof(AiState));
	state->cfg = cfg;
	XERR_REFLECT(List_Init(&state->reversedWaypointList, sizeof(Vec2I)));
	return XOK;
}

////////////////////////////////////////////////////////////////////////
/////////////////////////////// CHARACTER //////////////////////////////
////////////////////////////////////////////////////////////////////////

XErr CharacterState_Init(CharacterState* state, const CfgCharacter* cfg) {
	memset(state, 0, sizeof(struct CharacterState));
	state->cfg = cfg;
	if (cfg->defaultRangedWeapon) {
		XERR_REFLECT(RangedWeaponState_Init(&state->rangedWeaponState, cfg->defaultRangedWeapon));
	}
	if (cfg->defaultMeleeWeapon) {
		XERR_REFLECT(MeleeWeaponState_Init(&state->meleeWeaponState, cfg->defaultMeleeWeapon));
	}
	if (cfg->defaultExplosiveWeapon) {
		XERR_REFLECT(ExplosiveWeaponState_Init(&state->explosiveWeaponState, cfg->defaultExplosiveWeapon));
	}
	state->explosiveCount = cfg->defaultExplosiveCount;
	return XOK;
}

////////////////////////////////////////////////////////////////////////
//////////////////////////////// MARKUP ////////////////////////////////
////////////////////////////////////////////////////////////////////////

DEFINE_SIBLING_LIST_LENGTH_CALCULATOR(CfgMarkupElement)
