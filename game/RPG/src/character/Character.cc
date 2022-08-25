#include "rpg/Character.h"
#include <m2g/SpriteBlueprint.h>

chr::CharacterState::CharacterState(const CharacterBlueprint* blueprint) :
	blueprint(blueprint), dash_cooldown_counter_s(blueprint->dash_cooldown_s), stun_ttl_s() {
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

void chr::CharacterState::process_time(float time_passed_s) {
	if (explosive_weapon_state) {
		explosive_weapon_state->process_time(time_passed_s);
	}
	if (melee_weapon_state) {
		melee_weapon_state->process_time(time_passed_s);
	}
	if (ranged_weapon_state) {
		ranged_weapon_state->process_time(time_passed_s);
	}
	// Dash
	dash_cooldown_counter_s += time_passed_s;
	if (blueprint->dash_cooldown_s < dash_cooldown_counter_s) {
		dash_cooldown_counter_s = blueprint->dash_cooldown_s + 0.001f;
	}
	// Stun
	if (0.0f < stun_ttl_s) {
		stun_ttl_s -= time_passed_s;
		if (stun_ttl_s < 0.0f) {
			stun_ttl_s = 0.0f;
		}
	}
}

bool chr::CharacterState::pop_dash() {
	if (blueprint->dash_cooldown_s <= dash_cooldown_counter_s) {
		dash_cooldown_counter_s = 0.0f;
		return true;
	} else {
		return false;
	}
}

void chr::CharacterState::stun() {
	stun_ttl_s = blueprint->stun_ttl_s;
}
bool chr::CharacterState::is_stunned() const {
	return 0.0f < stun_ttl_s;
}

const m2::fsm::AnimationFSMBlueprint player_animation_fsm_blueprint = {
	.frames_per_second = 10,
	.states = {
		m2::fsm::AnimationFSMBlueprint::State{
			.state = chr::CHARANIMSTATE_STOP,
			.sprites = {m2g::IMPL_SPRITE_PLAYER_LOOKDOWN_00}
		},
		m2::fsm::AnimationFSMBlueprint::State{
			.state = chr::CHARANIMSTATE_WALKDOWN,
			.sprites = {
				m2g::IMPL_SPRITE_PLAYER_LOOKDOWN_00,
	            m2g::IMPL_SPRITE_PLAYER_LOOKDOWN_01,
	            m2g::IMPL_SPRITE_PLAYER_LOOKDOWN_00,
	            m2g::IMPL_SPRITE_PLAYER_LOOKDOWN_02
			}
		},
		m2::fsm::AnimationFSMBlueprint::State{
			.state = chr::CHARANIMSTATE_WALKRIGHT,
			.sprites = {
				m2g::IMPL_SPRITE_PLAYER_LOOKRIGHT_00,
				m2g::IMPL_SPRITE_PLAYER_LOOKRIGHT_01,
				m2g::IMPL_SPRITE_PLAYER_LOOKRIGHT_00,
				m2g::IMPL_SPRITE_PLAYER_LOOKRIGHT_02
			}
		},
		m2::fsm::AnimationFSMBlueprint::State{
			.state = chr::CHARANIMSTATE_WALKUP,
			.sprites = {
				m2g::IMPL_SPRITE_PLAYER_LOOKUP_00,
				m2g::IMPL_SPRITE_PLAYER_LOOKUP_01,
				m2g::IMPL_SPRITE_PLAYER_LOOKUP_00,
				m2g::IMPL_SPRITE_PLAYER_LOOKUP_02
			}
		},
		m2::fsm::AnimationFSMBlueprint::State{
			.state = chr::CHARANIMSTATE_WALKLEFT,
			.sprites = {
				m2g::IMPL_SPRITE_PLAYER_LOOKLEFT_00,
				m2g::IMPL_SPRITE_PLAYER_LOOKLEFT_01,
				m2g::IMPL_SPRITE_PLAYER_LOOKLEFT_00,
				m2g::IMPL_SPRITE_PLAYER_LOOKLEFT_02
			}
		}
	}
};

const chr::CharacterBlueprint chr::character_player = {
		.main_sprite_index = m2g::IMPL_SPRITE_PLAYER_LOOKDOWN_00,
		.mass_kg = 80.0f,
		.linear_damping = 100.0f,
		.walk_force = 2800.0f,
		.dash_force = 100000.0f,
		.max_hp = 100.0f,
		.default_explosive_weapon = &explosive_weapon_grenade,
		.default_melee_weapon = &melee_weapon_bat,
		.default_ranged_weapon = &ranged_weapon_gun,
		.dash_cooldown_s = 2.0f,
		.sprite_indexes = {
				m2g::IMPL_SPRITE_PLAYER_LOOKDOWN_00, // CFG_CHARTEXTURETYP_LOOKDOWN_00
				m2g::IMPL_SPRITE_PLAYER_LOOKDOWN_01, // CFG_CHARTEXTURETYP_LOOKDOWN_01
				m2g::IMPL_SPRITE_PLAYER_LOOKDOWN_02, // CFG_CHARTEXTURETYP_LOOKDOWN_02
				m2g::IMPL_SPRITE_PLAYER_LOOKLEFT_00, // CFG_CHARTEXTURETYP_LOOKLEFT_00
				m2g::IMPL_SPRITE_PLAYER_LOOKLEFT_01, // CFG_CHARTEXTURETYP_LOOKLEFT_01
				m2g::IMPL_SPRITE_PLAYER_LOOKLEFT_02, // CFG_CHARTEXTURETYP_LOOKLEFT_02
				m2g::IMPL_SPRITE_PLAYER_LOOKRIGHT_00, // CFG_CHARTEXTURETYP_LOOKRIGHT_00
				m2g::IMPL_SPRITE_PLAYER_LOOKRIGHT_01, // CFG_CHARTEXTURETYP_LOOKRIGHT_01
				m2g::IMPL_SPRITE_PLAYER_LOOKRIGHT_02, // CFG_CHARTEXTURETYP_LOOKRIGHT_02
				m2g::IMPL_SPRITE_PLAYER_LOOKUP_00, // CFG_CHARTEXTURETYP_LOOKUP_00
				m2g::IMPL_SPRITE_PLAYER_LOOKUP_01, // CFG_CHARTEXTURETYP_LOOKUP_01
				m2g::IMPL_SPRITE_PLAYER_LOOKUP_02, // CFG_CHARTEXTURETYP_LOOKUP_02
		},
		.animation_fsm_blueprint = &player_animation_fsm_blueprint
};
const chr::CharacterBlueprint chr::character_skeleton_000_chase = {
		.main_sprite_index = m2g::IMPL_SPRITE_ENEMY_LOOKDOWN_00,
		.mass_kg = 10.0f,
		.linear_damping = 10.0f,
		.walk_force = 25.0f,
		.max_hp = 100.0f,
		.default_explosive_weapon = nullptr,
		.default_melee_weapon = &melee_weapon_sword,
		.default_ranged_weapon = nullptr,
		.stun_ttl_s = 2.0f,
		.sprite_indexes = {
				m2g::IMPL_SPRITE_ENEMY_LOOKDOWN_00, // CFG_CHARTEXTURETYP_LOOKDOWN_00
				m2g::IMPL_SPRITE_ENEMY_LOOKDOWN_01, // CFG_CHARTEXTURETYP_LOOKDOWN_01
				m2g::IMPL_SPRITE_ENEMY_LOOKDOWN_02, // CFG_CHARTEXTURETYP_LOOKDOWN_02
				m2g::IMPL_SPRITE_ENEMY_LOOKLEFT_00, // CFG_CHARTEXTURETYP_LOOKLEFT_00
				m2g::IMPL_SPRITE_ENEMY_LOOKLEFT_01, // CFG_CHARTEXTURETYP_LOOKLEFT_01
				m2g::IMPL_SPRITE_ENEMY_LOOKLEFT_02, // CFG_CHARTEXTURETYP_LOOKLEFT_02
				m2g::IMPL_SPRITE_ENEMY_LOOKRIGHT_00, // CFG_CHARTEXTURETYP_LOOKRIGHT_00
				m2g::IMPL_SPRITE_ENEMY_LOOKRIGHT_01, // CFG_CHARTEXTURETYP_LOOKRIGHT_01
				m2g::IMPL_SPRITE_ENEMY_LOOKRIGHT_02, // CFG_CHARTEXTURETYP_LOOKRIGHT_02
				m2g::IMPL_SPRITE_ENEMY_LOOKUP_00, // CFG_CHARTEXTURETYP_LOOKUP_00
				m2g::IMPL_SPRITE_ENEMY_LOOKUP_01, // CFG_CHARTEXTURETYP_LOOKUP_01
				m2g::IMPL_SPRITE_ENEMY_LOOKUP_02, // CFG_CHARTEXTURETYP_LOOKUP_02
		},
		.aiBlueprint = &ai::chase_000
};
