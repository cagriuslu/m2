#include "../Cfg.h"

const CfgRangedWeapon CFG_RANGEDWPN_GUN = {
	.projectileTexture = &CFG_OBJTXTR_BULLET000,
	.projectileSpeed = 2.0f,
	.projectileDamage = 35.0f,
	.rateOfFire = 3.0f,
	.projectileCount = 1,
	.projectileTtl = 2.5f,
};
const CfgRangedWeapon CFG_RANGEDWPN_MACHINEGUN = {
	.projectileTexture = &CFG_OBJTXTR_BULLET001,
	.projectileSpeed = 2.5f,
	.projectileDamage = 35.0f,
	.rateOfFire = 12.0f,
	.projectileCount = 1,
	.projectileTtl = 3.0f,
};
const CfgRangedWeapon CFG_RANGEDWPN_SHOTGUN = {
	.projectileTexture = &CFG_OBJTXTR_BULLET000,
	.projectileSpeed = 2.0f,
	.projectileDamage = 20.0f,
	.rateOfFire = 1.0f,
	.projectileCount = 5,
	.projectileTtl = 3.0f,
};
const CfgRangedWeapon CFG_RANGEDWPN_BOWNARRAY = {
	.projectileTexture = &CFG_OBJTXTR_ARROW000,
	.projectileSpeed = 1.5f,
	.projectileDamage = 70.0f,
	.rateOfFire = 1.0f,
	.projectileCount = 1,
	.projectileTtl = 5.0f,
};

const CfgMeleeWeapon CFG_MELEEWPN_BAT = {
	.texture = &CFG_OBJTXTR_SWORD000,
	.meleeDamage = 60.0f,
	.rateOfFire = 1.0f,
	.meleeMotion = MELEE_MOTION_SWING,
};
const CfgMeleeWeapon CFG_MELEEWPN_SWORD = {
	.texture = &CFG_OBJTXTR_SWORD000,
	.meleeDamage = 90.0f,
	.rateOfFire = 1.0f,
	.meleeMotion = MELEE_MOTION_SWING,
};
const CfgMeleeWeapon CFG_MELEEWPN_SPEAR = {
	.texture = &CFG_OBJTXTR_SWORD000,
	.meleeDamage = 120.0f,
	.rateOfFire = 1.0f,
	.meleeMotion = MELEE_MOTION_STAB,
};
const CfgMeleeWeapon CFG_MELEEWPN_DAGGER = {
	.texture = &CFG_OBJTXTR_SWORD000,
	.meleeDamage = 60.0f,
	.rateOfFire = 2.0f,
	.meleeMotion = MELEE_MOTION_STAB,
};

const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADE = {
	.projectileTexture = &CFG_OBJTXTR_BULLET001,
	.projectileSpeed = 1.5f,
	.projectileDamageMax = 100.0f,
	.projectileDamageMin = 15.0f,
	.projectileDamageRadius = 2.0f,
	.rateOfFire = 1.0f,
};
const CfgExplosiveWeapon CFG_EXPLOSIVEWPN_GRENADELAUNCHER = {
	.projectileTexture = &CFG_OBJTXTR_BULLET000,
	.projectileSpeed = 2.5f,
	.projectileDamageMax =80.0f,
	.projectileDamageMin = 35.0f,
	.projectileDamageRadius = 0.75f,
	.rateOfFire = 1.0f,
};
