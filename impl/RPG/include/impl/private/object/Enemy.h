#ifndef IMPL_ENEMY_H
#define IMPL_ENEMY_H

#include <m2/Object.h>
#include "impl/private/ARPG_Cfg.hh"
#include "impl/private/fsm/Chaser.h"
#include "impl/private/fsm/DistanceKeeper.h"
#include "impl/private/fsm/HitNRunner.h"
#include "impl/private/fsm/Patroller.h"

struct EnemyData {
    CharacterState characterState;
    Automaton charAnimationAutomaton;
    float onHitColorModTtl;
};
#define AS_ENEMYDATA(ptr) ((EnemyData*)(ptr))

namespace impl::object {
    struct Enemy : public m2::ObjectImpl {
		using FSMVariant = std::variant<
			m2::FSM<impl::fsm::Chaser>,
			m2::FSM<impl::fsm::DistanceKeeper>,
			m2::FSM<impl::fsm::HitNRunner>,
			m2::FSM<impl::fsm::Patroller>
		>;
		FSMVariant fsmVariant;

        explicit Enemy(m2::Object&, const CfgCharacter* cfg);

        static M2Err init(m2::Object* obj, const CfgCharacter* cfg, m2::Vec2f pos);
    };
}

#endif //IMPL_ENEMY_H
