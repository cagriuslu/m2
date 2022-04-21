#include <rpg/character/Character.h>
#include <m2g/SpriteBlueprint.h>

impl::character::CharacterState::CharacterState(const CharacterBlueprint* blueprint) :
	blueprint(blueprint) {
	if (blueprint->default_explosive_weapon) {
		explosive_weapon_state = ExplosiveWeaponState(blueprint->default_explosive_weapon);
	}
	if (blueprint->default_melee_weapon) {
		melee_weapon_state = MeleeWeaponState(blueprint->default_melee_weapon);
	}
	if (blueprint->default_ranged_weapon) {
		ranged_weapon_state = RangedWeaponState(blueprint->default_ranged_weapon);
	}
}

void impl::character::CharacterState::process_time(float time_passed_s) {
	if (explosive_weapon_state) {
		explosive_weapon_state->process_time(time_passed_s);
	}
	if (melee_weapon_state) {
		melee_weapon_state->process_time(time_passed_s);
	}
	if (ranged_weapon_state) {
		ranged_weapon_state->process_time(time_passed_s);
	}
}

const impl::character::CharacterBlueprint impl::character::character_player = {
		.main_sprite_index = impl::IMPL_SPRITE_PLAYER_LOOKDOWN_00,
		.mass_kg = 4.0f,
		.linear_damping = 10.0f,
		.walk_speed = 25.0f,
		.max_hp = 100.0f,
		.default_explosive_weapon = &explosive_weapon_grenade,
		.default_melee_weapon = &melee_weapon_bat,
		.default_ranged_weapon = &ranged_weapon_gun,
		.sprite_indexes = {
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
const impl::character::CharacterBlueprint impl::character::character_skeleton_000_chase = {
		.main_sprite_index = impl::IMPL_SPRITE_ENEMY_LOOKDOWN_00,
		.mass_kg = 10.0f,
		.linear_damping = 10.0f,
		.walk_speed = 25.0f,
		.max_hp = 100.0f,
		.default_explosive_weapon = nullptr,
		.default_melee_weapon = &melee_weapon_sword,
		.default_ranged_weapon = nullptr,
		.sprite_indexes = {
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
		.aiBlueprint = &impl::ai::chase_000
};
