#pragma once
#include <Animation.pb.h>
#include <m2g_SpriteType.pb.h>

#include <vector>

#include "m2/Fsm.h"
#include "m2/Object.h"

namespace m2 {
	class Animation {
		pb::Animation _animation;
		std::vector<pb::AnimationState> _states;

	public:
		Animation() = default;
		explicit Animation(pb::Animation pb);
		const pb::Animation& animation() const { return _animation; }
		const pb::AnimationState& state(m2g::pb::AnimationStateType anim_state_type) const { return _states[pb::enum_index(anim_state_type)]; }
	};
}
