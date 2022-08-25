#ifndef IMPL_CHASE_HH
#define IMPL_CHASE_HH

#include "rpg/ai/type/Chase.h"
#include "m2/Object.h"
#include "m2/FSM.h"
#include <functional>
#include "rpg/ai/AiBlueprint.h"
#include <list>

namespace fsm {
    struct Chaser {
		// Inputs
        m2::Object& obj;
		const ai::AiBlueprint* blueprint;
		// State
		m2::Vec2f home_position;
		std::list<m2::Vec2i> reverse_waypoints;
		// Convenience
		m2::Object& target;
		m2::comp::Physique& phy;

        Chaser(m2::Object& obj, const ai::AiBlueprint* blueprint);

        static void* idle(m2::FSM<Chaser>& automaton, unsigned signal);
        static void* triggered(m2::FSM<Chaser>& automaton, unsigned signal);
        static void* gave_up(m2::FSM<Chaser>& automaton, unsigned signal);

        static constexpr auto initial_state = &idle;
    };
}

#endif //IMPL_CHASE_HH
