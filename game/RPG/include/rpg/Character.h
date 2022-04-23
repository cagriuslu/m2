#ifndef IMPL_CHARACTER_H
#define IMPL_CHARACTER_H

#include "ExplosiveWeapon.h"
#include "MeleeWeapon.h"
#include "RangedWeapon.h"
#include "rpg/ai/AiBlueprint.h"
#include "m2/SpriteBlueprint.h"
#include <optional>

namespace chr {
	enum CharacterTextureType {
		CHARACTER_TEXTURE_TYPE_LOOKDOWN_00,
		CHARACTER_TEXTURE_TYPE_LOOKDOWN_01,
		CHARACTER_TEXTURE_TYPE_LOOKDOWN_02,
		CHARACTER_TEXTURE_TYPE_LOOKLEFT_00,
		CHARACTER_TEXTURE_TYPE_LOOKLEFT_01,
		CHARACTER_TEXTURE_TYPE_LOOKLEFT_02,
		CHARACTER_TEXTURE_TYPE_LOOKRIGHT_00,
		CHARACTER_TEXTURE_TYPE_LOOKRIGHT_01,
		CHARACTER_TEXTURE_TYPE_LOOKRIGHT_02,
		CHARACTER_TEXTURE_TYPE_LOOKUP_00,
		CHARACTER_TEXTURE_TYPE_LOOKUP_01,
		CHARACTER_TEXTURE_TYPE_LOOKUP_02,
		CHARACTER_TEXTURE_TYPE_N
	};

	struct CharacterBlueprint;
	struct CharacterState {
		const CharacterBlueprint* blueprint;
		std::optional<ExplosiveWeaponState> explosive_weapon_state;
		std::optional<MeleeWeaponState> melee_weapon_state;
		std::optional<RangedWeaponState> ranged_weapon_state;
		explicit CharacterState(const CharacterBlueprint* blueprint);
		void process_time(float time_passed_s);
	};
	struct CharacterBlueprint {
		m2::SpriteIndex main_sprite_index;
		float mass_kg;
		float linear_damping;
		float walk_force;
		float dash_force;
		float max_hp;
		const ExplosiveWeaponBlueprint* default_explosive_weapon;
		const MeleeWeaponBlueprint* default_melee_weapon;
		const RangedWeaponBlueprint* default_ranged_weapon;
		m2::SpriteIndex sprite_indexes[CHARACTER_TEXTURE_TYPE_N];
		const ai::AiBlueprint* aiBlueprint;
	};

	extern const CharacterBlueprint character_player;
	extern const CharacterBlueprint character_skeleton_000_chase;
}

#endif //IMPL_CHARACTER_H
