#include "../Cfg.h"

const CfgRangedWeapon CFG_RANGEDWPN_GUN = {
	.projectile = {
		.texture = &CFG_OBJTXTR_BULLET_00,
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
		.texture = &CFG_OBJTXTR_BULLET_00,
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
		.texture = &CFG_OBJTXTR_BULLET_01,
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
		.texture = &CFG_OBJTXTR_BULLET_01,
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

const CfgMeleeWeapon CFG_MELEEWPN_BAT = {
	.melee = {
		.texture = &CFG_OBJTXTR_SWORD_00,
		.damage = 60.0f,
		.motion = CFG_MELEE_MOTION_SWING,
		.ttl_s = 0.150f
	},
	.cooldown_s = 0.33f
};
const CfgMeleeWeapon CFG_MELEEWPN_SWORD = {
	.melee = {
		.texture = &CFG_OBJTXTR_SWORD_00,
		.damage = 45.0f, // TODO normally 90
		.motion = CFG_MELEE_MOTION_SWING,
		.ttl_s = 0.150f
	},
	.cooldown_s = 0.33f
};
const CfgMeleeWeapon CFG_MELEEWPN_SPEAR = {
	.melee = {
		.texture = &CFG_OBJTXTR_SWORD_00,
		.damage = 120.0f,
		.motion = CFG_MELEE_MOTION_STAB,
		.ttl_s = 0.150f
	},
	.cooldown_s = 1.0f
};
const CfgMeleeWeapon CFG_MELEEWPN_DAGGER = {
	.melee = {
		.texture = &CFG_OBJTXTR_SWORD_00,
		.damage = 60.0f,
		.motion = CFG_MELEE_MOTION_STAB,
		.ttl_s = 0.150f
	},
	.cooldown_s = 0.5f
};

const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADE = {
	.explosive = {
		.texture = &CFG_OBJTXTR_BOMB_00,
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
		.texture = &CFG_OBJTXTR_BOMB_00,
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
