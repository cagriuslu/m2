#include <m2/Animation.h>
#include <m2/Proto.h>
#include <m2/Exception.h>

m2::Animation::Animation(const pb::Animation &pb) : _animation(pb) {
	_states.resize(m2g::pb::AnimationStateType_ARRAYSIZE);
	for (const auto& state : _animation.states()) {
		_states[state.type()] = state;
	}
}

std::vector<m2::Animation> m2::load_animations(const std::string& path) {
	auto animations = proto::json_file_to_message<pb::Animations>(path);
	if (!animations) {
		throw M2ERROR(animations.error());
	}
	std::vector<Animation> animations_vector;
	std::for_each(animations->animations().begin(), animations->animations().end(), [&](const auto& animation) {
		animations_vector.emplace_back(animation);
	});
	return animations_vector;
}
