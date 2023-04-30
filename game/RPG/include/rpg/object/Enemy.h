#ifndef IMPL_ENEMY_H
#define IMPL_ENEMY_H

#include <m2/fsm/AnimationFsm.h>
#include "m2/Object.h"
#include "rpg/fsm/Chaser.h"
#include "rpg/fsm/DistanceKeeper.h"
#include "rpg/fsm/HitNRunner.h"
#include "rpg/fsm/Patroller.h"
#include <m2/Value.h>
#include <variant>

namespace rpg {
    struct Enemy : public m2::ObjectImpl {
		m2::AnimationFsm animation_fsm;

		using AiFsmVariant = std::variant<
			std::monostate,
			rpg::ChaserFsm,
			rpg::DistanceKeeperFsm,
			rpg::HitNRunnerFsm,
			rpg::PatrollerFsm
		>;
		AiFsmVariant ai_fsm;

		Enemy(m2::Object& obj, const rpg::pb::Enemy* enemy);

        static m2::VoidValue init(m2::Object& obj, m2g::pb::ObjectType object_type);
		static void move_to(m2::Object& obj, const m2::Vec2f& target, float force);
		static void move_towards(m2::Object& obj, const m2::Vec2f& direction, float force);
    };
}

#endif //IMPL_ENEMY_H
