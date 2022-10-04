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
		explicit Animation(const pb::Animation& pb);
		const pb::Animation& animation() const { return _animation; }
		const pb::AnimationState& state(m2g::pb::AnimationStateType anim_state_type) const { return _states[anim_state_type]; }
	};
	std::vector<Animation> load_animations(const std::string& path);

	class AnimationFsmBase {
		const Animation& animation;
		GraphicId gfx_id;
		std::pair<m2g::pb::AnimationStateType,int> state_sprite_pair;

	public:
		AnimationFsmBase(m2g::pb::AnimationType animation_type, GraphicId gfx_id);

	protected:
		static void* state_func(Fsm<AnimationFsmBase>& fsm, int signal);
		static constexpr auto initial_state = &state_func;
	};
}

#endif //M2_ANIMATION_H
