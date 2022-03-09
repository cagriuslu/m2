#include "../Cfg.h"

const CfgCharacter CFG_CHARACTER_PLAYER = {
	.mainTexture = &CFG_OBJTXTR_PLAYER_LOOKDOWN_00,
	.objType = CFG_OBJTYP_PLAYER,
	.mass_kg = 4.0f,
	.linearDamping = 10.0f,
	.walkSpeed = 25.0f,
	.maxHp = 100.0f,
	.defaultRangedWeapon = &CFG_RANGEDWPN_GUN,
	.defaultMeleeWeapon = &CFG_MELEEWPN_BAT,
	.defaultExplosiveWeapon = &CFG_EXPLOSIVEWPN_GRENADE,
	.textures = {
		&CFG_OBJTXTR_PLAYER_LOOKDOWN_00, // CFG_CHARTEXTURETYP_LOOKDOWN_00
		&CFG_OBJTXTR_PLAYER_LOOKDOWN_01, // CFG_CHARTEXTURETYP_LOOKDOWN_01
		&CFG_OBJTXTR_PLAYER_LOOKDOWN_02, // CFG_CHARTEXTURETYP_LOOKDOWN_02
		&CFG_OBJTXTR_PLAYER_LOOKLEFT_00, // CFG_CHARTEXTURETYP_LOOKLEFT_00
		&CFG_OBJTXTR_PLAYER_LOOKLEFT_01, // CFG_CHARTEXTURETYP_LOOKLEFT_01
		&CFG_OBJTXTR_PLAYER_LOOKLEFT_02, // CFG_CHARTEXTURETYP_LOOKLEFT_02
		&CFG_OBJTXTR_PLAYER_LOOKRIGHT_00, // CFG_CHARTEXTURETYP_LOOKRIGHT_00
		&CFG_OBJTXTR_PLAYER_LOOKRIGHT_01, // CFG_CHARTEXTURETYP_LOOKRIGHT_01
		&CFG_OBJTXTR_PLAYER_LOOKRIGHT_02, // CFG_CHARTEXTURETYP_LOOKRIGHT_02
		&CFG_OBJTXTR_PLAYER_LOOKUP_00, // CFG_CHARTEXTURETYP_LOOKUP_00
		&CFG_OBJTXTR_PLAYER_LOOKUP_01, // CFG_CHARTEXTURETYP_LOOKUP_01
		&CFG_OBJTXTR_PLAYER_LOOKUP_02, // CFG_CHARTEXTURETYP_LOOKUP_02
	}
};
const CfgCharacter CFG_CHARACTER_SKELETON_000_CHASE = {
	.mainTexture = &CFG_OBJTXTR_ENEMY_LOOKDOWN_00,
	.objType = CFG_OBJTYP_ENEMY,
	.mass_kg = 10.0f,
	.linearDamping = 10.0f,
	.walkSpeed = 25.0f,
	.maxHp = 100.0f,
	.defaultRangedWeapon = NULL,
	.defaultMeleeWeapon = &CFG_MELEEWPN_SWORD,
	.defaultExplosiveWeapon = NULL,
	.textures = {
		&CFG_OBJTXTR_ENEMY_LOOKDOWN_00, // CFG_CHARTEXTURETYP_LOOKDOWN_00
		&CFG_OBJTXTR_ENEMY_LOOKDOWN_01, // CFG_CHARTEXTURETYP_LOOKDOWN_01
		&CFG_OBJTXTR_ENEMY_LOOKDOWN_02, // CFG_CHARTEXTURETYP_LOOKDOWN_02
		&CFG_OBJTXTR_ENEMY_LOOKLEFT_00, // CFG_CHARTEXTURETYP_LOOKLEFT_00
		&CFG_OBJTXTR_ENEMY_LOOKLEFT_01, // CFG_CHARTEXTURETYP_LOOKLEFT_01
		&CFG_OBJTXTR_ENEMY_LOOKLEFT_02, // CFG_CHARTEXTURETYP_LOOKLEFT_02
		&CFG_OBJTXTR_ENEMY_LOOKRIGHT_00, // CFG_CHARTEXTURETYP_LOOKRIGHT_00
		&CFG_OBJTXTR_ENEMY_LOOKRIGHT_01, // CFG_CHARTEXTURETYP_LOOKRIGHT_01
		&CFG_OBJTXTR_ENEMY_LOOKRIGHT_02, // CFG_CHARTEXTURETYP_LOOKRIGHT_02
		&CFG_OBJTXTR_ENEMY_LOOKUP_00, // CFG_CHARTEXTURETYP_LOOKUP_00
		&CFG_OBJTXTR_ENEMY_LOOKUP_01, // CFG_CHARTEXTURETYP_LOOKUP_01
		&CFG_OBJTXTR_ENEMY_LOOKUP_02, // CFG_CHARTEXTURETYP_LOOKUP_02
	},
	.ai = &CFG_AI_CHASE_00
};
const CfgCharacter CFG_CHARACTER_SKELETON_000_KEEP_DISTANCE = {
	.mainTexture = &CFG_OBJTXTR_ENEMY_LOOKDOWN_00,
	.objType = CFG_OBJTYP_ENEMY,
	.mass_kg = 10.0f,
	.linearDamping = 10.0f,
	.walkSpeed = 25.0f,
	.maxHp = 100.0f,
	.defaultRangedWeapon = &CFG_RANGEDWPN_GUN,
	.defaultMeleeWeapon = NULL,
	.defaultExplosiveWeapon = NULL,
	.textures = {
		&CFG_OBJTXTR_ENEMY_LOOKDOWN_00, // CFG_CHARTEXTURETYP_LOOKDOWN_00
		&CFG_OBJTXTR_ENEMY_LOOKDOWN_01, // CFG_CHARTEXTURETYP_LOOKDOWN_01
		&CFG_OBJTXTR_ENEMY_LOOKDOWN_02, // CFG_CHARTEXTURETYP_LOOKDOWN_02
		&CFG_OBJTXTR_ENEMY_LOOKLEFT_00, // CFG_CHARTEXTURETYP_LOOKLEFT_00
		&CFG_OBJTXTR_ENEMY_LOOKLEFT_01, // CFG_CHARTEXTURETYP_LOOKLEFT_01
		&CFG_OBJTXTR_ENEMY_LOOKLEFT_02, // CFG_CHARTEXTURETYP_LOOKLEFT_02
		&CFG_OBJTXTR_ENEMY_LOOKRIGHT_00, // CFG_CHARTEXTURETYP_LOOKRIGHT_00
		&CFG_OBJTXTR_ENEMY_LOOKRIGHT_01, // CFG_CHARTEXTURETYP_LOOKRIGHT_01
		&CFG_OBJTXTR_ENEMY_LOOKRIGHT_02, // CFG_CHARTEXTURETYP_LOOKRIGHT_02
		&CFG_OBJTXTR_ENEMY_LOOKUP_00, // CFG_CHARTEXTURETYP_LOOKUP_00
		&CFG_OBJTXTR_ENEMY_LOOKUP_01, // CFG_CHARTEXTURETYP_LOOKUP_01
		&CFG_OBJTXTR_ENEMY_LOOKUP_02, // CFG_CHARTEXTURETYP_LOOKUP_02
	},
	.ai = &CFG_AI_KEEP_DISTANCE_00
};
const CfgCharacter CFG_CHARACTER_SKELETON_000_HIT_N_RUN = {
	.mainTexture = &CFG_OBJTXTR_ENEMY_LOOKDOWN_00,
	.objType = CFG_OBJTYP_ENEMY,
	.mass_kg = 10.0f,
	.linearDamping = 10.0f,
	.walkSpeed = 25.0f,
	.maxHp = 100.0f,
	.defaultRangedWeapon = NULL,
	.defaultMeleeWeapon = &CFG_MELEEWPN_SWORD,
	.defaultExplosiveWeapon = NULL,
	.textures = {
		&CFG_OBJTXTR_ENEMY_LOOKDOWN_00, // CFG_CHARTEXTURETYP_LOOKDOWN_00
		&CFG_OBJTXTR_ENEMY_LOOKDOWN_01, // CFG_CHARTEXTURETYP_LOOKDOWN_01
		&CFG_OBJTXTR_ENEMY_LOOKDOWN_02, // CFG_CHARTEXTURETYP_LOOKDOWN_02
		&CFG_OBJTXTR_ENEMY_LOOKLEFT_00, // CFG_CHARTEXTURETYP_LOOKLEFT_00
		&CFG_OBJTXTR_ENEMY_LOOKLEFT_01, // CFG_CHARTEXTURETYP_LOOKLEFT_01
		&CFG_OBJTXTR_ENEMY_LOOKLEFT_02, // CFG_CHARTEXTURETYP_LOOKLEFT_02
		&CFG_OBJTXTR_ENEMY_LOOKRIGHT_00, // CFG_CHARTEXTURETYP_LOOKRIGHT_00
		&CFG_OBJTXTR_ENEMY_LOOKRIGHT_01, // CFG_CHARTEXTURETYP_LOOKRIGHT_01
		&CFG_OBJTXTR_ENEMY_LOOKRIGHT_02, // CFG_CHARTEXTURETYP_LOOKRIGHT_02
		&CFG_OBJTXTR_ENEMY_LOOKUP_00, // CFG_CHARTEXTURETYP_LOOKUP_00
		&CFG_OBJTXTR_ENEMY_LOOKUP_01, // CFG_CHARTEXTURETYP_LOOKUP_01
		&CFG_OBJTXTR_ENEMY_LOOKUP_02, // CFG_CHARTEXTURETYP_LOOKUP_02
	},
	.ai = &CFG_AI_HIT_N_RUN_00
};
