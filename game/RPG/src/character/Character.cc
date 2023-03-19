#include "rpg/Character.h"

const chr::CharacterBlueprint chr::character_player = {
		.main_sprite = m2g::pb::SpriteType::PLAYER_LOOKDOWN_00,
		.animation_type = m2g::pb::ANIMATION_TYPE_PLAYER_MOVEMENT
};

const chr::CharacterBlueprint chr::character_skeleton_000_chase = {
		.main_sprite = m2g::pb::SpriteType::ENEMY_LOOKDOWN_00,
		.animation_type = m2g::pb::ANIMATION_TYPE_SKELETON_MOVEMENT,
		.aiBlueprint = &ai::chase_000,
};
