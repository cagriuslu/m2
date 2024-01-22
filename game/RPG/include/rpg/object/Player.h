#pragma once
#include <m2/Animation.h>
#include <m2/Meta.h>
#include <m2g_ItemType.pb.h>

#include <vector>

#include "m2/Object.h"
#include "m2/game/fsm/AnimationFsm.h"

namespace rpg {
	struct Player : public m2::ObjectImpl {
		m2::AnimationFsm animation_fsm;

		explicit Player(m2::Object& obj);

		static m2::void_expected init(m2::Object& obj);
	};
}
