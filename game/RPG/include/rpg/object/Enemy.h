#pragma once
#include <m2/fsm/AnimationFsm.h>
#include "m2/Object.h"
#include "rpg/fsm/Chaser.h"
#include "rpg/fsm/Escaper.h"
#include "rpg/fsm/HitNRunner.h"
#include "rpg/fsm/Patroller.h"
#include <m2/Meta.h>
#include <variant>

namespace rpg {
    struct Enemy : public m2::ObjectImpl {
		m2::AnimationFsm animation_fsm;

		using AiFsmVariant = std::variant<
			std::monostate,
			rpg::ChaserFsm,
			rpg::EscaperFsm,
			rpg::HitNRunnerFsm,
			rpg::PatrollerFsm
		>;
		AiFsmVariant ai_fsm;

		Enemy(m2::Object& obj, const rpg::pb::Enemy* enemy);

        static m2::void_expected init(m2::Object& obj, m2g::pb::ObjectType object_type);
		static void move_towards(m2::Object& obj, m2::VecF direction, float force);
		static void attack_if_close(m2::Object& obj, const pb::Ai& ai);
    };
}
