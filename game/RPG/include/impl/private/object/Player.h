#ifndef IMPL_PLAYER_H
#define IMPL_PLAYER_H

#include <m2/Object.h>
#include <impl/private/character/Character.h>
#include <impl/private/fsm/CharacterAnimation.h>

namespace impl::object {
	struct Player : public m2::ObjectImpl {
		character::CharacterState char_state;
		m2::FSM<impl::fsm::CharacterAnimation> char_animator;

		Player(m2::Object&, const character::CharacterBlueprint*);

		static M2Err init(m2::Object& obj, const character::CharacterBlueprint* blueprint, m2::Vec2f pos);
	};
}

#endif //IMPL_PLAYER_H
