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
		explicit CharacterState(const CharacterBlueprint* blueprint);
	};
	struct CharacterBlueprint {
		m2g::pb::SpriteType main_sprite;
		m2g::pb::AnimationType animation_type;
		const ai::AiBlueprint* aiBlueprint;
	};

	extern const CharacterBlueprint character_player;
	extern const CharacterBlueprint character_skeleton_000_chase;
}

#endif //IMPL_CHARACTER_H
