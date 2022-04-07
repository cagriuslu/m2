#include "impl/private/ARPG_Cfg.hh"
#include <impl/public/SpriteBlueprint.h>
#include <m2/Vec2i.hh>

////////////////////////////////////////////////////////////////////////
///////////////////// RANGED WEAPON AND PROJECTILE /////////////////////
////////////////////////////////////////////////////////////////////////
const CfgRangedWeapon CFG_RANGEDWPN_GUN = {
		.projectile = {
				.spriteIndex = impl::IMPL_SPRITE_BULLET_00,
				.speed_mps = 20.0f,
				.damage = 35.0f,
				.ttl_s = 0.6f,
				.damageAccuracy = 0.8f,
				.ttlAccuracy = 0.90f
		},
		.projectileCount = 1,
		.cooldown_s = 0.10f,
		.accuracy = 0.95f
};
const CfgRangedWeapon CFG_RANGEDWPN_MACHINEGUN = {
		.projectile = {
				.spriteIndex = impl::IMPL_SPRITE_BULLET_00,
				.speed_mps = 2.5f,
				.damage = 35.0f,
				.ttl_s = 3.0f,
				.damageAccuracy = 0.8f,
				.ttlAccuracy = 0.95f
		},
		.projectileCount = 1,
		.cooldown_s = 0.083f,
		.accuracy = 0.9f
};
const CfgRangedWeapon CFG_RANGEDWPN_SHOTGUN = {
		.projectile = {
				.spriteIndex = impl::IMPL_SPRITE_BULLET_01,
				.speed_mps = 2.0f,
				.damage = 20.0f,
				.ttl_s = 3.0f,
				.damageAccuracy = 0.8f,
				.ttlAccuracy = 0.95f
		},
		.projectileCount = 5,
		.cooldown_s = 1.0f,
		.accuracy = 0.9f
};
const CfgRangedWeapon CFG_RANGEDWPN_BOW = {
		.projectile = {
				.spriteIndex = impl::IMPL_SPRITE_BULLET_01,
				.speed_mps = 1.5f,
				.damage = 70.0f,
				.ttl_s = 5.0f,
				.damageAccuracy = 0.8f,
				.ttlAccuracy = 0.95f
		},
		.projectileCount = 1,
		.cooldown_s = 1.0f,
		.accuracy = 0.9f
};

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
const CfgMeleeWeapon CFG_MELEEWPN_BAT = {
		.melee = {
				.spriteIndex = impl::IMPL_SPRITE_SWORD_00,
				.damage = 60.0f,
				.motion = CFG_MELEE_MOTION_SWING,
				.ttl_s = 0.150f
		},
		.cooldown_s = 0.33f
};
const CfgMeleeWeapon CFG_MELEEWPN_SWORD = {
		.melee = {
				.spriteIndex = impl::IMPL_SPRITE_SWORD_00,
				.damage = 45.0f, // TODO normally 90
				.motion = CFG_MELEE_MOTION_SWING,
				.ttl_s = 0.150f
		},
		.cooldown_s = 0.33f
};
const CfgMeleeWeapon CFG_MELEEWPN_SPEAR = {
		.melee = {
				.spriteIndex = impl::IMPL_SPRITE_SWORD_00,
				.damage = 120.0f,
				.motion = CFG_MELEE_MOTION_STAB,
				.ttl_s = 0.150f
		},
		.cooldown_s = 1.0f
};
const CfgMeleeWeapon CFG_MELEEWPN_DAGGER = {
		.melee = {
				.spriteIndex = impl::IMPL_SPRITE_SWORD_00,
				.damage = 60.0f,
				.motion = CFG_MELEE_MOTION_STAB,
				.ttl_s = 0.150f
		},
		.cooldown_s = 0.5f
};

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
const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADE = {
		.explosive = {
				.spriteIndex = impl::IMPL_SPRITE_BOMB_00,
				.projectileSpeed_mps = 1.5f,
				.projectileTtl_s = 4.0f,
				.projectileBodyRadius_m = 0.25f,
				.damageMax = 100.0f,
				.damageMin = 15.0f,
				.damageRadius_m = 2.0f
		},
		.cooldown_s = 1.0f,
		.initialExplosiveCount = 5
};
const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADELAUNCHER = {
		.explosive = {
				.spriteIndex = impl::IMPL_SPRITE_BOMB_00,
				.projectileSpeed_mps = 2.5f,
				.projectileTtl_s = 2.0f,
				.projectileBodyRadius_m = 0.25f,
				.damageMax = 80.0f,
				.damageMin = 35.0f,
				.damageRadius_m = 0.75f
		},
		.cooldown_s = 1.0f,
		.initialExplosiveCount = 5
};

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
const CfgAi CFG_AI_CHASE_00 = {
		.behavior = CFG_AI_BEHAVIOR_CHASE,
		.capability = CFG_AI_CAPABILITY_MELEE,
		.triggerDistanceSquared_m = 25.0f,
		.attackDistanceSquared_m = 0.5625f,
		.giveUpDistanceSquared_m = 100.0f,
		.recalculationPeriod_s = 0.75f,
};

M2Err AiState_Init(AiState *state, const CfgAi* cfg, m2::Vec2f homePosition) {
	state->cfg = cfg;
	state->homePosition = homePosition;
	return M2OK;
}

////////////////////////////////////////////////////////////////////////
/////////////////////////////// CHARACTER //////////////////////////////
////////////////////////////////////////////////////////////////////////
const CfgCharacter CFG_CHARACTER_PLAYER = {
		.mainSpriteIndex = impl::IMPL_SPRITE_PLAYER_LOOKDOWN_00,
		.objType = CFG_OBJTYP_PLAYER,
		.mass_kg = 4.0f,
		.linearDamping = 10.0f,
		.walkSpeed = 25.0f,
		.maxHp = 100.0f,
		.defaultRangedWeapon = &CFG_RANGEDWPN_GUN,
		.defaultMeleeWeapon = &CFG_MELEEWPN_BAT,
		.defaultExplosiveWeapon = &CFG_EXPLOSIVEWPN_GRENADE,
		.spriteIndexes = {
				impl::IMPL_SPRITE_PLAYER_LOOKDOWN_00, // CFG_CHARTEXTURETYP_LOOKDOWN_00
				impl::IMPL_SPRITE_PLAYER_LOOKDOWN_01, // CFG_CHARTEXTURETYP_LOOKDOWN_01
				impl::IMPL_SPRITE_PLAYER_LOOKDOWN_02, // CFG_CHARTEXTURETYP_LOOKDOWN_02
				impl::IMPL_SPRITE_PLAYER_LOOKLEFT_00, // CFG_CHARTEXTURETYP_LOOKLEFT_00
				impl::IMPL_SPRITE_PLAYER_LOOKLEFT_01, // CFG_CHARTEXTURETYP_LOOKLEFT_01
				impl::IMPL_SPRITE_PLAYER_LOOKLEFT_02, // CFG_CHARTEXTURETYP_LOOKLEFT_02
				impl::IMPL_SPRITE_PLAYER_LOOKRIGHT_00, // CFG_CHARTEXTURETYP_LOOKRIGHT_00
				impl::IMPL_SPRITE_PLAYER_LOOKRIGHT_01, // CFG_CHARTEXTURETYP_LOOKRIGHT_01
				impl::IMPL_SPRITE_PLAYER_LOOKRIGHT_02, // CFG_CHARTEXTURETYP_LOOKRIGHT_02
				impl::IMPL_SPRITE_PLAYER_LOOKUP_00, // CFG_CHARTEXTURETYP_LOOKUP_00
				impl::IMPL_SPRITE_PLAYER_LOOKUP_01, // CFG_CHARTEXTURETYP_LOOKUP_01
				impl::IMPL_SPRITE_PLAYER_LOOKUP_02, // CFG_CHARTEXTURETYP_LOOKUP_02
		}
};
const CfgCharacter CFG_CHARACTER_SKELETON_000_CHASE = {
		.mainSpriteIndex = impl::IMPL_SPRITE_ENEMY_LOOKDOWN_00,
		.objType = CFG_OBJTYP_ENEMY,
		.mass_kg = 10.0f,
		.linearDamping = 10.0f,
		.walkSpeed = 25.0f,
		.maxHp = 100.0f,
		.defaultRangedWeapon = NULL,
		.defaultMeleeWeapon = &CFG_MELEEWPN_SWORD,
		.defaultExplosiveWeapon = NULL,
		.spriteIndexes = {
				impl::IMPL_SPRITE_ENEMY_LOOKDOWN_00, // CFG_CHARTEXTURETYP_LOOKDOWN_00
				impl::IMPL_SPRITE_ENEMY_LOOKDOWN_01, // CFG_CHARTEXTURETYP_LOOKDOWN_01
				impl::IMPL_SPRITE_ENEMY_LOOKDOWN_02, // CFG_CHARTEXTURETYP_LOOKDOWN_02
				impl::IMPL_SPRITE_ENEMY_LOOKLEFT_00, // CFG_CHARTEXTURETYP_LOOKLEFT_00
				impl::IMPL_SPRITE_ENEMY_LOOKLEFT_01, // CFG_CHARTEXTURETYP_LOOKLEFT_01
				impl::IMPL_SPRITE_ENEMY_LOOKLEFT_02, // CFG_CHARTEXTURETYP_LOOKLEFT_02
				impl::IMPL_SPRITE_ENEMY_LOOKRIGHT_00, // CFG_CHARTEXTURETYP_LOOKRIGHT_00
				impl::IMPL_SPRITE_ENEMY_LOOKRIGHT_01, // CFG_CHARTEXTURETYP_LOOKRIGHT_01
				impl::IMPL_SPRITE_ENEMY_LOOKRIGHT_02, // CFG_CHARTEXTURETYP_LOOKRIGHT_02
				impl::IMPL_SPRITE_ENEMY_LOOKUP_00, // CFG_CHARTEXTURETYP_LOOKUP_00
				impl::IMPL_SPRITE_ENEMY_LOOKUP_01, // CFG_CHARTEXTURETYP_LOOKUP_01
				impl::IMPL_SPRITE_ENEMY_LOOKUP_02, // CFG_CHARTEXTURETYP_LOOKUP_02
		},
		.ai = &CFG_AI_CHASE_00
};

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
