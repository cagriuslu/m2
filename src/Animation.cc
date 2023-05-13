#include <m2/Animation.h>
#include <m2/protobuf/Detail.h>
#include <m2/Exception.h>
#include <utility>

using namespace m2g::pb;

m2::Animation::Animation(pb::Animation pb) : _animation(std::move(pb)) {
	_states.resize(protobuf::enum_value_count<AnimationStateType>());
	for (const auto& state : _animation.states()) {
		_states[protobuf::enum_index(state.type())] = state;
	}
}

std::vector<m2::Animation> m2::load_animations(const std::filesystem::path& path) {
	auto animations = protobuf::json_file_to_message<pb::Animations>(path);
	if (!animations) {
		throw M2ERROR(animations.error());
	}

	std::vector<Animation> animations_vector(protobuf::enum_value_count<AnimationType>());
	std::vector<bool> is_loaded(protobuf::enum_value_count<AnimationType>());

	// Load animations
	for (const auto& animation : animations->animations()) {
		auto index = protobuf::enum_index(animation.type());
		// Check if animation is already loaded
		if (is_loaded[index]) {
			throw M2ERROR("Animation has duplicate definition: " + AnimationType_Name(animation.type()));
		}
		// Load animation
		animations_vector[index] = Animation{animation};
		is_loaded[index] = true;
	}

	// Check if every animation is loaded
	for (int i = 0; i < protobuf::enum_value_count<AnimationType>(); ++i) {
		if (!is_loaded[i]) {
			throw M2ERROR("Animation is not defined: " + protobuf::enum_name<AnimationType>(i));
		}
	}

	return animations_vector;
}
