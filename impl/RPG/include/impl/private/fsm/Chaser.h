#ifndef IMPL_CHASE_HH
#define IMPL_CHASE_HH

#include <impl/private/ai/type/Chase.h>
#include <m2/Object.h>
#include <m2/FSM.h>
#include <functional>

namespace impl::fsm {
    struct Chaser {
		// Inputs
        m2::Object& obj;
		const ai::AiBlueprint* blueprint;
		// State
		m2::Vec2f home_position;
		std::list<m2::Vec2i> reverse_waypoints;
		// Convenience
		m2::Object& target;
		m2::component::Physique& phy;

        Chaser(m2::Object& obj, const ai::AiBlueprint* blueprint);

        static void* idle(m2::FSM<Chaser>& automaton, int sig);
        static void* triggered(m2::FSM<Chaser>& automaton, int sig);
        static void* gave_up(m2::FSM<Chaser>& automaton, int sig);

        static constexpr auto initial_state = &idle;
    };
}

#endif //IMPL_CHASE_HH
