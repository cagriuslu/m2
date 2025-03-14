#pragma once
#include <Animation.pb.h>

#include "m2/game/Fsm.h"
#include "m2/Object.h"
#include "m2/game/Animation.h"

namespace m2 {
	class AnimationFsmState {};

	class AnimationFsmSignal : public FsmSignalBase {
		m2g::pb::AnimationStateType _animation_state;
	public:
		explicit AnimationFsmSignal(m2g::pb::AnimationStateType animation_state) : FsmSignalBase(FsmSignalType::Custom), _animation_state(animation_state) {}
		using FsmSignalBase::FsmSignalBase;

		[[nodiscard]] m2g::pb::AnimationStateType animation_state_type() const { return _animation_state; }
	};

	class AnimationFsm : public FsmBase<AnimationFsmState, AnimationFsmSignal> {
		const Animation& animation;
		const GraphicId gfx_id;
		m2g::pb::AnimationStateType anim_state;
		int state_index;

	public:
		AnimationFsm(m2g::pb::AnimationType animation_type, GraphicId gfx_id);

	protected:
		std::optional<AnimationFsmState> HandleSignal(const AnimationFsmSignal& signal) override;
	};
}
