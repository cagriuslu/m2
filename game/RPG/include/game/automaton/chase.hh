#ifndef GAME_CHASE_HH
#define GAME_CHASE_HH

#include <m2/object.hh>
#include <m2/automaton.hh>
#include <functional>

namespace game::automaton::chase {
    class Data {
        m2::object::Object& obj;
        m2::object::Object& target;
        Physique& phy;

    public:
        Data(m2::object::Object& obj, m2::object::Object& target);

        static void* idle(m2::automaton::Automaton<Data>& automaton, int sig);
        static void* triggered(m2::automaton::Automaton<Data>& automaton, int sig);
        static void* gave_up(m2::automaton::Automaton<Data>& automaton, int sig);

        static constexpr auto initial_state = &idle;
    };
}

#endif //GAME_CHASE_HH
