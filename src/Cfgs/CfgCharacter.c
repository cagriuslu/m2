#include "../Cfg.h"

const CfgCharacter CFG_CHARACTER_PLAYER = {
	.texture = &CFG_OBJTXTR_PLAYER000,
	.objType = CFG_OBJTYP_PLAYER,
	.walkSpeed = 0.8f,
	.maxHp = 100.0f,
	.defaultRangedWeapon = &CFG_RANGEDWPN_GUN,
	.defaultMeleeWeapon = &CFG_MELEEWPN_BAT,
	.defaultExplosiveWeapon = &CFG_EXPLOSIVEWPN_GRENADE,
	.defaultExplosiveCount = 4,
};
const CfgCharacter CFG_CHARACTER_SKELETON000 = {
	.texture = &CFG_OBJTXTR_SKELETON000,
	.objType = CFG_OBJTYP_ENEMY,
	.walkSpeed = 0.55f,
	.maxHp = 100.0f,
	.defaultRangedWeapon = NULL,
	.defaultMeleeWeapon = &CFG_MELEEWPN_SWORD,
	.defaultExplosiveWeapon = NULL,
	.defaultExplosiveCount = 0,
};
