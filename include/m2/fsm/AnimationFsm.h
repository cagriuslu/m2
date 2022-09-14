#ifndef M2_FSM_ANIMATIONFSM_H
#define M2_FSM_ANIMATIONFSM_H

#include <m2g/SpriteBlueprint.h>
#include "../Fsm.h"
#include "../Object.h"
#include "../Game.hh"
#include "../Log.h"
#include <memory>
#include <optional>

namespace m2::fsm {
	// Do not create this object directly, use make_animation_fsm_blueprint
	struct AnimationFsmBlueprint {
		struct State {
			unsigned state;
			std::vector<SpriteIndex> sprites;
		};

		float frames_per_second;
		std::vector<State> states;
	};
	AnimationFsmBlueprint make_animation_fsm_blueprint(AnimationFsmBlueprint&& val);

	/// AnimationFSM actually have only one state
	/// It would be better if it was an actor, instead of FSM
	class AnimationFsmData {
		// Input
		const AnimationFsmBlueprint* blueprint;
		GraphicId gfx_id;
		// Data
		std::pair<unsigned,unsigned> state_sprite_idx;

		AnimationFsmData(const AnimationFsmBlueprint* blueprint, GraphicId gfx_id);

		// States
		static FsmStateHandler state_func(Fsm<AnimationFsmData>& automaton, unsigned signal);
		static constexpr auto initial_state = &state_func;

		friend class AnimationFsm;
		friend class Fsm<AnimationFsmData>;
	};

	class AnimationFsm : private Fsm<AnimationFsmData> {
	public:
		AnimationFsm(const AnimationFsmBlueprint* blueprint, GraphicId gfx_id);
		void set_state(unsigned state);
		using Fsm<AnimationFsmData>::time;
	};
}

#endif //M2_FSM_ANIMATIONFSM_H
