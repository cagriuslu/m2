#ifndef IMPL_PLAYER_H
#define IMPL_PLAYER_H

#include "m2/Object.h"
#include <m2/Animation.h>
#include "m2/fsm/AnimationFsm.h"
#include <vector>
#include <m2/Value.h>
#include <ItemType.pb.h>

namespace rpg {
	struct Player : public m2::ObjectImpl {
		m2::AnimationFsm animation_fsm;

		explicit Player(m2::Object& obj);

		static m2::VoidValue init(m2::Object& obj);
	};
}

#endif //IMPL_PLAYER_H
