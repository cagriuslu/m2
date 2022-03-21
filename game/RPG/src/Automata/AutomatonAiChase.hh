#ifndef GAME_AUTOMATON_AI_CHASE_HH
#define GAME_AUTOMATON_AI_CHASE_HH

#include <m2/object/Object.hh>
#include <m2/Automaton.hh>

namespace game {
    struct AutomatonAiChase : public m2::Automaton {
        Object& obj;
        ComponentPhysique& phy;

        AutomatonAiChase(Object&);
    };
}

#endif //GAME_AUTOMATON_AI_CHASE_HH
