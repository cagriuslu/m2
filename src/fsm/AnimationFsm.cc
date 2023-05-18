#include <m2/fsm/AnimationFsm.h>
#include <m2/Game.h>

m2::AnimationFsm::AnimationFsm(m2g::pb::AnimationType animation_type, GraphicId gfx_id) : FsmBase({}), animation(GAME.animations[animation_type]), gfx_id(gfx_id), anim_state(GAME.animations[animation_type].animation().initial_state()), state_index(0) {
	init();
}

std::optional<m2::AnimationFsmState> m2::AnimationFsm::handle_signal(const AnimationFsmSignal& signal) {
	auto set_sprite = [this](m2g::pb::AnimationStateType state, int index) {
		this->anim_state = state;
		this->state_index = index;
		auto gfx = LEVEL.graphics.get(this->gfx_id);
		if (gfx) {
			gfx->sprite = &GAME.get_sprite(animation.state(state).sprites(index));
		} else {
			LOG_WARN("Graphics component destroyed before Animation FSM");
		}
	};

	if (signal.type() ==  FsmSignalType::EnterState) {
		set_sprite(anim_state, state_index);
		arm(1.0f / animation.animation().fps());
	} else if (signal.type() == FsmSignalType::Alarm) {
		auto state_sprite_count = animation.state(anim_state).sprites_size();
		set_sprite(anim_state, (state_index + 1) % state_sprite_count);
		arm(1.0f / animation.animation().fps());
	} else if (signal.type() == FsmSignalType::Custom) {
		// Check if the state change is necessary
		if (anim_state != signal.animation_state_type()) {
			// Check if there are sprites in the state
			if (animation.state(signal.animation_state_type()).sprites_size()) {
				set_sprite(signal.animation_state_type(), 0);
				arm(1.0f / animation.animation().fps());
			} else {
				// Else, change the animation state, but disarm the alarm
				anim_state = signal.animation_state_type();
				state_index = 0;
				disarm();
			}
		}
	}
	return {};
}
