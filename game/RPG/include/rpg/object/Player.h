#ifndef IMPL_PLAYER_H
#define IMPL_PLAYER_H

#include "m2/Object.h"
#include <m2/Animation.h>
#include "rpg/Character.h"
#include <vector>
#include <m2/Value.h>
#include <ItemType.pb.h>

namespace obj {
	struct Player : public m2::ObjectImpl {
		chr::CharacterState char_state;
		m2::Fsm<m2::AnimationFsmBase> animation_fsm;

		Player(m2::Object&, const chr::CharacterBlueprint*);

		static m2::VoidValue init(m2::Object& obj, const chr::CharacterBlueprint* blueprint);
	};
}

#endif //IMPL_PLAYER_H
