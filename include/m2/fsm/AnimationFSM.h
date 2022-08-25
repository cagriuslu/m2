#ifndef M2_FSM_ANIMATIONFSM_H
#define M2_FSM_ANIMATIONFSM_H

#include <m2g/SpriteBlueprint.h>
#include "../FSM.h"
#include "../Object.h"
#include "../Game.hh"
#include "../Log.h"
#include <memory>
#include <optional>

namespace m2::fsm {
	struct AnimationFSMBlueprint {
		struct State {
			unsigned state;
			std::vector<SpriteIndex> sprites;
		};

		float frames_per_second;
		std::vector<State> states;
	};

	// Forward declaration
	class AnimationFSM;

	/// AnimationFSM actually have only one state
	/// It would be better if it was an actor, instead of FSM
	class AnimationFSMData {
		// Input
		const AnimationFSMBlueprint* blueprint;
		GraphicID gfx_id;
		// Data
		std::pair<unsigned,unsigned> state_sprite_idx;

		AnimationFSMData(const AnimationFSMBlueprint* blueprint, GraphicID gfx_id);

		// States
		static void* state_func(FSM<AnimationFSMData>& automaton, unsigned signal);

		static constexpr auto initial_state = &state_func;

		friend class AnimationFSM;
		friend struct FSM<AnimationFSMData>;
	};

	class AnimationFSM : private FSM<AnimationFSMData> {
	public:
		AnimationFSM(const AnimationFSMBlueprint* blueprint, GraphicID gfx_id);
		void set_state(unsigned state);
		using FSM<AnimationFSMData>::time;
	};
}

#endif //M2_FSM_ANIMATIONFSM_H
