#include <m2/Animation.h>
#include <m2/protobuf/Utils.h>
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

	std::vector<Animation> animations_vector(m2g::pb::AnimationType_ARRAYSIZE);
	std::vector<bool> is_loaded(m2g::pb::AnimationType_ARRAYSIZE);

	// Load animations
	for (const auto& animation : animations->animations()) {
		// Check if animation is already loaded
		if (is_loaded[animation.type()]) {
			throw M2ERROR("Animation has duplicate definition: " + std::to_string(animation.type()));
		}
		// Load animation
		animations_vector[animation.type()] = Animation{animation};
		is_loaded[animation.type()] = true;
	}

	// Check if every animation is loaded
	const auto* animation_type_desc = m2g::pb::AnimationType_descriptor();
	for (int e = 0; e < animation_type_desc->value_count(); ++e) {
		int value = animation_type_desc->value(e)->number();
		if (!is_loaded[value]) {
			throw M2ERROR("Animation is not defined: " + std::to_string(value));
		}
	}

	return animations_vector;
}
