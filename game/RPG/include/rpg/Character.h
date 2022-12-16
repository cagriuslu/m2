#ifndef IMPL_CHARACTER_H
#define IMPL_CHARACTER_H

#include "ExplosiveWeapon.h"
#include "MeleeWeapon.h"
#include "rpg/ai/AiBlueprint.h"
#include <AnimationType.pb.h>
#include <optional>

namespace chr {
	struct CharacterBlueprint;
	struct CharacterState {
		const CharacterBlueprint* blueprint;
		std::optional<ExplosiveWeaponState> explosive_weapon_state;
		std::optional<MeleeWeaponState> melee_weapon_state;
		float stun_ttl_s;
		explicit CharacterState(const CharacterBlueprint* blueprint);
		void process_time(float time_passed_s);
		// Stun
		void stun();
		[[nodiscard]] bool is_stunned() const;
	};
	struct CharacterBlueprint {
		m2g::pb::SpriteType main_sprite;
		const ExplosiveWeaponBlueprint* default_explosive_weapon;
		const MeleeWeaponBlueprint* default_melee_weapon;
		float stun_ttl_s;
		m2g::pb::AnimationType animation_type;
		const ai::AiBlueprint* aiBlueprint;
	};

	extern const CharacterBlueprint character_player;
	extern const CharacterBlueprint character_skeleton_000_chase;
}

#endif //IMPL_CHARACTER_H
