#pragma once
#include <AnimationStateType.pb.h>
#include <m2/game/CharacterMovement.h>

namespace rpg::detail {
	m2g::pb::AnimationStateType to_animation_state_type(m2::CharacterMovementDirection dir);
}
