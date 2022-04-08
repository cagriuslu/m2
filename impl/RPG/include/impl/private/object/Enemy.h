#ifndef IMPL_ENEMY_H
#define IMPL_ENEMY_H

#include <m2/Object.h>
#include "impl/private/ARPG_Cfg.hh"
#include "impl/private/fsm/Chaser.h"

struct EnemyData {
    CharacterState characterState;
    Automaton charAnimationAutomaton;
    AiState aiState;
    Automaton aiAutomaton;
    float onHitColorModTtl;
};
#define AS_ENEMYDATA(ptr) ((EnemyData*)(ptr))

namespace impl::object {
    struct Enemy : public m2::ObjectImpl {
        m2::FSM<impl::fsm::Chaser> chaser;

        explicit Enemy(m2::Object&);
    };
}

int ObjectEnemy_InitFromCfg(m2::Object* obj, const CfgCharacter *cfg, m2::Vec2f position);


#endif //IMPL_ENEMY_H
