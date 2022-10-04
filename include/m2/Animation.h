#ifndef M2_ANIMATION_H
#define M2_ANIMATION_H

#include <SpriteType.pb.h>
#include <Animation.pb.h>
#include <vector>

namespace m2 {
	class Animation {
		pb::Animation _animation;
		std::vector<pb::AnimationState> _states;

	public:
		explicit Animation(const pb::Animation& pb);
	};

	std::vector<Animation> load_animations(const std::string& path);
}

#endif //M2_ANIMATION_H
