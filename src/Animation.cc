#include <m2/Game.h>
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

m2::AnimationFsmBase::AnimationFsmBase(m2g::pb::AnimationType animation_type, GraphicId gfx_id) : animation(GAME.animations[animation_type]), gfx_id(gfx_id), state_sprite_pair(animation.animation().initial_state(), 0) {}
void* m2::AnimationFsmBase::state_func(Fsm<AnimationFsmBase>& fsm, int signal) {
	// Determine next sprite index
	auto animation = fsm.animation;
	auto curr_pair = fsm.state_sprite_pair;
	std::optional<decltype(curr_pair)> new_pair;
	if (signal == FSM_SIGNAL_ENTER) {
		new_pair = curr_pair;
		fsm.arm(1.0f / animation.animation().fps());
	} else if (signal == FSM_SIGNAL_ALARM) {
		auto curr_state_sprite_count = animation.state(curr_pair.first).sprites_size();
		auto new_sprite_idx = (curr_pair.second + 1 == curr_state_sprite_count) ? 0 : curr_pair.second + 1;
		new_pair = {curr_pair.first, new_sprite_idx};
		fsm.arm(1.0f / animation.animation().fps());
	} else if (FSM_SIGNAL_CUSTOM <= signal) {
		auto signal_state = static_cast<m2g::pb::AnimationStateType>(signal - FSM_SIGNAL_CUSTOM);
		if (signal_state != curr_pair.first) {
			new_pair = {signal_state, 0};
		}
	}

	// Change sprite if necessary
	if (new_pair) {
		auto gfx = GAME.graphics.get(fsm.gfx_id);
		if (gfx) {
			auto sprite_type = animation.state(new_pair->first).sprites(new_pair->second);
			gfx->sprite = &GAME.sprites[sprite_type];
		} else {
			LOG_WARN("Graphics component destroyed before Animation FSM");
		}
		fsm.state_sprite_pair = *new_pair;
	}

	return nullptr; // There is only one state :(
}
