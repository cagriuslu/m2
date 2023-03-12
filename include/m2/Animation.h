#ifndef M2_ANIMATION_H
#define M2_ANIMATION_H

#include "Fsm.h"
#include "Object.h"
#include <SpriteType.pb.h>
#include <Animation.pb.h>
#include <vector>

namespace m2 {
	class Animation {
		pb::Animation _animation;
		std::vector<pb::AnimationState> _states;

	public:
		Animation() = default;
		explicit Animation(pb::Animation pb);
		const pb::Animation& animation() const { return _animation; }
		const pb::AnimationState& state(m2g::pb::AnimationStateType anim_state_type) const { return _states[proto::enum_index(anim_state_type)]; }
	};
	std::vector<Animation> load_animations(const std::string& path);
}

#endif //M2_ANIMATION_H
