#ifndef IMPL_ENEMY_H
#define IMPL_ENEMY_H

#include "rpg/Character.h"
#include <m2/fsm/AnimationFsm.h>
#include "m2/Object.h"
#include "rpg/fsm/Chaser.h"
#include "rpg/fsm/DistanceKeeper.h"
#include "rpg/fsm/HitNRunner.h"
#include "rpg/fsm/Patroller.h"
#include <m2/Value.h>

namespace obj {
    struct Enemy : public m2::ObjectImpl {
		m2::AnimationFsm animation_fsm;

		using AiFsmVariant = std::variant<
			rpg::ChaserFsm,
			rpg::DistanceKeeperFsm,
			rpg::HitNRunnerFsm,
			rpg::PatrollerFsm
		>;
		AiFsmVariant ai_fsm;

		Enemy(m2::Object&, const chr::CharacterBlueprint*);

        static m2::VoidValue init(m2::Object& obj, const chr::CharacterBlueprint* blueprint);
    };
}

#endif //IMPL_ENEMY_H
