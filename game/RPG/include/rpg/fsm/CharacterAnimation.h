#ifndef IMPL_CHARACTERANIMATION_H
#define IMPL_CHARACTERANIMATION_H

#include "m2/FSM.h"
#include "rpg/character/Character.h"
#include "m2/component/Graphic.h"

namespace impl::fsm {
	struct CharacterAnimation {
		enum FSMSignal {
			CHARANIM_STOP = m2::FSMSIG_N,
			CHARANIM_WALKDOWN,
			CHARANIM_WALKRIGHT,
			CHARANIM_WALKUP,
			CHARANIM_WALKLEFT,
		};

		// Inputs
		m2::component::Graphic& gfx;
		const character::CharacterBlueprint* blueprint;

		CharacterAnimation(m2::component::Graphic& gfx, const character::CharacterBlueprint* blueprint);

		static void* idle(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* downStop(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* down00(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* down01(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* down02(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* down03(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* leftStop(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* left00(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* left01(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* left02(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* left03(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* rightStop(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* right00(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* right01(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* right02(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* right03(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* upStop(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* up00(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* up01(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* up02(m2::FSM<CharacterAnimation>& automaton, int signal);
		static void* up03(m2::FSM<CharacterAnimation>& automaton, int signal);

		static constexpr auto initial_state = &idle;
	};
}

#endif //IMPL_CHARACTERANIMATION_H
