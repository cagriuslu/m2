#ifndef IMPL_CHARACTER_H
#define IMPL_CHARACTER_H

#include "rpg/ai/AiBlueprint.h"
#include <SpriteType.pb.h>
#include <AnimationType.pb.h>
#include <optional>

namespace chr {
	struct CharacterBlueprint;
	struct CharacterState {
		const CharacterBlueprint* blueprint;
		float stun_ttl_s;
		explicit CharacterState(const CharacterBlueprint* blueprint);
		void process_time(float time_passed_s);
		// Stun
		void stun();
		[[nodiscard]] bool is_stunned() const;
	};
	struct CharacterBlueprint {
		m2g::pb::SpriteType main_sprite;
		float stun_ttl_s;
		m2g::pb::AnimationType animation_type;
		const ai::AiBlueprint* aiBlueprint;
	};

	extern const CharacterBlueprint character_player;
	extern const CharacterBlueprint character_skeleton_000_chase;
}

#endif //IMPL_CHARACTER_H
