#pragma once
#include <m2/Meta.h>

#include <variant>

#include "m2/Object.h"
#include "m2/game/fsm/AnimationFsm.h"
#include "rpg/fsm/Chaser.h"
#include "rpg/fsm/Escaper.h"
#include "rpg/fsm/HitNRunner.h"
#include "rpg/fsm/Patroller.h"
#include <m2/component/character/FastCharacter.h>

namespace rpg {
	class EnemyCharacter : public m2::FastCharacter {
	public:
		using FastCharacter::FastCharacter;

		m2::Object& GetOwner();

		void OnUpdate(m2::Stopwatch::Duration);
		void OnMessage(m2::Interaction);
	};

    struct Enemy : public m2::HeapObjectImpl {
    	EnemyCharacter& chr;
		m2::AnimationFsm animation_fsm;

		using AiFsmVariant = std::variant<
			std::monostate,
			rpg::ChaserFsm,
			rpg::EscaperFsm,
			rpg::HitNRunnerFsm,
			rpg::PatrollerFsm
		>;
		AiFsmVariant ai_fsm;

		Enemy(m2::Object& obj, EnemyCharacter& chr_, const rpg::pb::Enemy* enemy);

        static m2::void_expected init(m2::Object& obj, const m2::VecF& position);
		static void move_towards(m2::Object& obj, m2::VecF direction, float force);
		static void attack_if_close(m2::Object& obj, const pb::Ai& ai);
    };
}
