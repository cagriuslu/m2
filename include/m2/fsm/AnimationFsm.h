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

	class AnimationFsmBase {
		// Input
		const AnimationFsmBlueprint* blueprint;
		GraphicId gfx_id;
		// Data
		std::pair<unsigned,unsigned> state_sprite_idx;

	public:
		AnimationFsmBase(const AnimationFsmBlueprint* blueprint, GraphicId gfx_id);

	protected:
		static void* state_func(Fsm<AnimationFsmBase>& fsm, int signal);
		static constexpr auto initial_state = &state_func;
	};
}

#endif //M2_FSM_ANIMATIONFSM_H
