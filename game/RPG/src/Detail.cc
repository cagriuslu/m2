#include <rpg/Detail.h>

m2g::pb::AnimationStateType rpg::detail::CharacterMovementDirection_to_AnimationStateType(m2::CharacterMovementDirection dir) {
	auto animation_state_type = m2g::pb::ANIMATION_STATE_IDLE;
	switch (dir) {
		case m2::CHARMOVEMENT_DOWN_LEFT:
		case m2::CHARMOVEMENT_LEFT:
		case m2::CHARMOVEMENT_UP_LEFT:
			animation_state_type = m2g::pb::ANIMATION_STATE_WALKLEFT;
			break;
		case m2::CHARMOVEMENT_DOWN_RIGHT:
		case m2::CHARMOVEMENT_RIGHT:
		case m2::CHARMOVEMENT_UP_RIGHT:
			animation_state_type = m2g::pb::ANIMATION_STATE_WALKRIGHT;
			break;
		case m2::CHARMOVEMENT_DOWN:
			animation_state_type = m2g::pb::ANIMATION_STATE_WALKDOWN;
			break;
		case m2::CHARMOVEMENT_UP:
			animation_state_type = m2g::pb::ANIMATION_STATE_WALKUP;
			break;
		default:
			break;
	}
	return animation_state_type;
}
