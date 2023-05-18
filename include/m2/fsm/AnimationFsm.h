#ifndef M2_ANIMATIONFSM_H
#define M2_ANIMATIONFSM_H

#include "../Fsm.h"
#include "../Object.h"
#include "../Animation.h"
#include <Animation.pb.h>

namespace m2 {
	class AnimationFsmState {};

	class AnimationFsmSignal : public FsmSignalBase {
		m2g::pb::AnimationStateType _animation_state;
	public:
		inline explicit AnimationFsmSignal(m2g::pb::AnimationStateType animation_state) : FsmSignalBase(FsmSignalType::Custom), _animation_state(animation_state) {}
		using FsmSignalBase::FsmSignalBase;

		[[nodiscard]] inline m2g::pb::AnimationStateType animation_state_type() const { return _animation_state; }
	};

	class AnimationFsm : public FsmBase<AnimationFsmState, AnimationFsmSignal> {
		const Animation& animation;
		const GraphicId gfx_id;
		m2g::pb::AnimationStateType anim_state;
		int state_index;

	public:
		AnimationFsm(m2g::pb::AnimationType animation_type, GraphicId gfx_id);

	protected:
		std::optional<AnimationFsmState> handle_signal(const AnimationFsmSignal& signal) override;
	};
}

#endif //M2_ANIMATIONFSM_H
