#include "../Cfg.h"

const CfgRangedWeapon CFG_RANGEDWPN_GUN = {
	.projectile = {
		.texture = &CFG_OBJTXTR_BULLET000,
		.speed = 2.0f,
		.damage = 35.0f,
		.ttl = 2.5f
	},
	.projectileCount = 1,
	.rateOfFire = 3.0f
};
const CfgRangedWeapon CFG_RANGEDWPN_MACHINEGUN = {
	.projectile = {
		.texture = &CFG_OBJTXTR_BULLET001,
		.speed = 2.5f,
		.damage = 35.0f,
		.ttl = 3.0f
	},
	.projectileCount = 1,
	.rateOfFire = 12.0f
};
const CfgRangedWeapon CFG_RANGEDWPN_SHOTGUN = {
	.projectile = {
		.texture = &CFG_OBJTXTR_BULLET000,
		.speed = 2.0f,
		.damage = 20.0f,
		.ttl = 3.0f
	},
	.projectileCount = 5,
	.rateOfFire = 1.0f
};
const CfgRangedWeapon CFG_RANGEDWPN_BOW = {
	.projectile = {
		.texture = &CFG_OBJTXTR_ARROW000,
		.speed = 1.5f,
		.damage = 70.0f,
		.ttl = 5.0f
	},
	.projectileCount = 1,
	.rateOfFire = 1.0f
};

const CfgMeleeWeapon CFG_MELEEWPN_BAT = {
	.melee = {
		.texture = &CFG_OBJTXTR_SWORD000,
		.damage = 60.0f,
		.motion = CFG_MELEE_MOTION_SWING
	},
	.rateOfFire = 1.0f
};
const CfgMeleeWeapon CFG_MELEEWPN_SWORD = {
	.melee = {
		.texture = &CFG_OBJTXTR_SWORD000,
		.damage = 90.0f,
		.motion = CFG_MELEE_MOTION_SWING
	},
	.rateOfFire = 1.0f
};
const CfgMeleeWeapon CFG_MELEEWPN_SPEAR = {
	.melee = {
		.texture = &CFG_OBJTXTR_SWORD000,
		.damage = 120.0f,
		.motion = CFG_MELEE_MOTION_STAB
	},
	.rateOfFire = 1.0f
};
const CfgMeleeWeapon CFG_MELEEWPN_DAGGER = {
	.melee = {
		.texture = &CFG_OBJTXTR_SWORD000,
		.damage = 60.0f,
		.motion = CFG_MELEE_MOTION_STAB
	},
	.rateOfFire = 2.0f
};

const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADE = {
	.explosive = {
		.texture = &CFG_OBJTXTR_BULLET001,
		.speed = 1.5f,
		.damageMax = 100.0f,
		.damageMin = 15.0f,
		.damageRadius = 2.0f
	},
	.rateOfFire = 1.0f
};
const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADELAUNCHER = {
	.explosive = {
			.texture = &CFG_OBJTXTR_BULLET000,
			.speed = 2.5f,
			.damageMax = 80.0f,
			.damageMin = 35.0f,
			.damageRadius = 0.75f
	},
	.rateOfFire = 1.0f
};
