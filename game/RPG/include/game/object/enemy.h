#ifndef GAME_ENEMY_H
#define GAME_ENEMY_H

#include <m2/object.hh>
#include <game/ARPG_Cfg.hh>
#include <game/automaton/chase.hh>

struct EnemyData {
    CharacterState characterState;
    Automaton charAnimationAutomaton;
    AiState aiState;
    Automaton aiAutomaton;
    float onHitColorModTtl;
};
#define AS_ENEMYDATA(ptr) ((EnemyData*)(ptr))

namespace game::object::enemy {
    struct Data : public m2::object::Data {
        m2::automaton::Automaton<game::automaton::chase::Data> chaser;

        explicit Data(m2::object::Object&);
    };
}

int ObjectEnemy_InitFromCfg(m2::object::Object* obj, const CfgCharacter *cfg, m2::vec2f position);


#endif //GAME_ENEMY_H
