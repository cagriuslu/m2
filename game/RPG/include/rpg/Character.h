#ifndef IMPL_CHARACTER_H
#define IMPL_CHARACTER_H

#include <m2g/SpriteID.h>
#include "ExplosiveWeapon.h"
#include "MeleeWeapon.h"
#include "RangedWeapon.h"
#include "rpg/ai/AiBlueprint.h"
#include "m2/SpriteBlueprint.h"
#include <m2/fsm/AnimationFsm.h>
#include <optional>

namespace chr {
	enum CharacterAnimationState {
		CHARANIMSTATE_STOP,
		CHARANIMSTATE_WALKDOWN,
		CHARANIMSTATE_WALKRIGHT,
		CHARANIMSTATE_WALKUP,
		CHARANIMSTATE_WALKLEFT,
	};

	struct CharacterBlueprint;
	struct CharacterState {
		const CharacterBlueprint* blueprint;
		std::optional<ExplosiveWeaponState> explosive_weapon_state;
		std::optional<MeleeWeaponState> melee_weapon_state;
		std::optional<RangedWeaponState> ranged_weapon_state;
		float dash_cooldown_counter_s;
		float stun_ttl_s;
		explicit CharacterState(const CharacterBlueprint* blueprint);
		void process_time(float time_passed_s);
		// Dash
		bool pop_dash();
		// Stun
		void stun();
		[[nodiscard]] bool is_stunned() const;
	};
	struct CharacterBlueprint {
		m2::SpriteIndex main_sprite_index;
		m2g::SpriteID main_sprite;
		float mass_kg;
		float linear_damping;
		float walk_force;
		float dash_force;
		float max_hp;
		const ExplosiveWeaponBlueprint* default_explosive_weapon;
		const MeleeWeaponBlueprint* default_melee_weapon;
		const RangedWeaponBlueprint* default_ranged_weapon;
		float dash_cooldown_s;
		float stun_ttl_s;
		const m2::fsm::AnimationFsmBlueprint* animation_fsm_blueprint;
		const ai::AiBlueprint* aiBlueprint;
	};

	extern const CharacterBlueprint character_player;
	extern const CharacterBlueprint character_skeleton_000_chase;
}

#endif //IMPL_CHARACTER_H
