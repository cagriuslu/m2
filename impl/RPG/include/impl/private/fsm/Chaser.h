#ifndef IMPL_CHASE_HH
#define IMPL_CHASE_HH

#include "m2/Object.h"
#include "m2/FSM.h"
#include <functional>

namespace impl::fsm {
    class Chaser {
        m2::Object& obj;
        m2::Object& target;
        m2::component::Physique& phy;

    public:
        Chaser(m2::Object& obj, m2::Object& target);

        static void* idle(m2::FSM<Chaser>& automaton, int sig);
        static void* triggered(m2::FSM<Chaser>& automaton, int sig);
        static void* gave_up(m2::FSM<Chaser>& automaton, int sig);

        static constexpr auto initial_state = &idle;
    };
}

#endif //IMPL_CHASE_HH
