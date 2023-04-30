#ifndef RPG_DETAIL_H
#define RPG_DETAIL_H

#include <AnimationStateType.pb.h>
#include <m2/game/CharacterMovement.h>

namespace rpg::detail {
	m2g::pb::AnimationStateType CharacterMovementDirection_to_AnimationStateType(m2::CharacterMovementDirection dir);
}

#endif //RPG_DETAIL_H
