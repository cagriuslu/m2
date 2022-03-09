#include "Cfg.h"

////////////////////////////////////////////////////////////////////////
///////////////////// RANGED WEAPON AND PROJECTILE /////////////////////
////////////////////////////////////////////////////////////////////////

M2Err ProjectileState_Init(ProjectileState* state, const CfgProjectile* cfg) {
	memset(state, 0, sizeof(ProjectileState));
	state->cfg = cfg;
	state->ttl_s = cfg->ttl_s;
	return M2OK;
}

M2Err RangedWeaponState_Init(RangedWeaponState* state, const CfgRangedWeapon* cfg) {
	memset(state, 0, sizeof(RangedWeaponState));
	state->cfg = cfg;
	state->cooldownCounter_s = cfg->cooldown_s;
	return M2OK;
}

void RangedWeaponState_ProcessTime(RangedWeaponState* state, float timePassed) {
	state->cooldownCounter_s += timePassed;
	if (state->cfg->cooldown_s < state->cooldownCounter_s) {
		state->cooldownCounter_s = state->cfg->cooldown_s + timePassed;
	}
}

////////////////////////////////////////////////////////////////////////
//////////////////////// MELEE WEAPON AND MELEE ////////////////////////
////////////////////////////////////////////////////////////////////////

M2Err MeleeState_Init(MeleeState* state, const CfgMelee* cfg) {
	memset(state, 0, sizeof(MeleeState));
	state->cfg = cfg;
	state->ttl_s = cfg->ttl_s;
	return M2OK;
}

M2Err MeleeWeaponState_Init(MeleeWeaponState* state, const CfgMeleeWeapon* cfg) {
	memset(state, 0, sizeof(MeleeWeaponState));
	state->cfg = cfg;
	state->cooldownCounter_s = cfg->cooldown_s;
	return M2OK;
}

void MeleeWeaponState_ProcessTime(MeleeWeaponState* state, float timePassed) {
	state->cooldownCounter_s += timePassed;
	if (state->cfg->cooldown_s < state->cooldownCounter_s) {
		state->cooldownCounter_s = state->cfg->cooldown_s + timePassed;
	}
}

////////////////////////////////////////////////////////////////////////
//////////////////// EXPLOSIVE WEAPON AND EXPLOSIVE ////////////////////
////////////////////////////////////////////////////////////////////////

M2Err ExplosiveState_Init(ExplosiveState* state, const CfgExplosive* cfg) {
	memset(state, 0, sizeof(ExplosiveState));
	state->cfg = cfg;
	state->projectileTtl_s = cfg->projectileTtl_s;
	return M2OK;
}

M2Err ExplosiveWeaponState_Init(ExplosiveWeaponState* state, const CfgExplosiveWeapon* cfg) {
	memset(state, 0, sizeof(ExplosiveWeaponState));
	state->cfg = cfg;
	state->cooldownCounter_s = cfg->cooldown_s;
	state->explosiveCount = cfg->initialExplosiveCount;
	return M2OK;
}

void ExplosiveWeaponState_ProcessTime(ExplosiveWeaponState* state, float timePassed) {
	state->cooldownCounter_s += timePassed;
	if (state->cfg->cooldown_s < state->cooldownCounter_s) {
		state->cooldownCounter_s = state->cfg->cooldown_s + timePassed;
	}
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////// AI //////////////////////////////////
////////////////////////////////////////////////////////////////////////

M2Err AiState_Init(AiState *state, const CfgAi* cfg, Vec2F homePosition) {
	memset(state, 0, sizeof(AiState));
	state->cfg = cfg;
	state->homePosition = homePosition;
	M2ERR_REFLECT(List_Init(&state->reversedWaypointList, 10, sizeof(Vec2I)));
	return M2OK;
}

////////////////////////////////////////////////////////////////////////
/////////////////////////////// CHARACTER //////////////////////////////
////////////////////////////////////////////////////////////////////////

M2Err CharacterState_Init(CharacterState* state, const CfgCharacter* cfg) {
	memset(state, 0, sizeof(struct CharacterState));
	state->cfg = cfg;
	if (cfg->defaultRangedWeapon) {
		M2ERR_REFLECT(RangedWeaponState_Init(&state->rangedWeaponState, cfg->defaultRangedWeapon));
	}
	if (cfg->defaultMeleeWeapon) {
		M2ERR_REFLECT(MeleeWeaponState_Init(&state->meleeWeaponState, cfg->defaultMeleeWeapon));
	}
	if (cfg->defaultExplosiveWeapon) {
		M2ERR_REFLECT(ExplosiveWeaponState_Init(&state->explosiveWeaponState, cfg->defaultExplosiveWeapon));
	}
	return M2OK;
}

void CharacterState_ProcessTime(CharacterState* state, float timePassed) {
	if (state->rangedWeaponState.cfg) {
		RangedWeaponState_ProcessTime(&state->rangedWeaponState, timePassed);
	}
	if (state->meleeWeaponState.cfg) {
		MeleeWeaponState_ProcessTime(&state->meleeWeaponState, timePassed);
	}
	if (state->explosiveWeaponState.cfg) {
		ExplosiveWeaponState_ProcessTime(&state->explosiveWeaponState, timePassed);
	}
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////// UI //////////////////////////////////
////////////////////////////////////////////////////////////////////////

DEFINE_SIBLING_LIST_LENGTH_CALCULATOR(CfgUIElement)
