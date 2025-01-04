#include "m2/game/fsm/AnimationFsm.h"
#include "m2/Game.h"
#include "m2/Log.h"

m2::AnimationFsm::AnimationFsm(const m2g::pb::AnimationType animation_type, const GraphicId gfx_id)
		: FsmBase(), animation(M2_GAME.animations[animation_type]), gfx_id(gfx_id),
		anim_state(M2_GAME.animations[animation_type].animation().initial_state()), state_index(0) {
	init({});
}

std::optional<m2::AnimationFsmState> m2::AnimationFsm::HandleSignal(const AnimationFsmSignal& signal) {
	auto set_sprite = [this](const m2g::pb::AnimationStateType state, const int index) {
		this->anim_state = state;
		this->state_index = index;
		if (auto* gfx = M2_LEVEL.graphics.get(this->gfx_id)) {
			gfx->visual = &std::get<Sprite>(M2_GAME.GetSpriteOrTextLabel(animation.state(state).sprites(index)));
		} else {
			LOG_WARN("Graphics component destroyed before Animation FSM");
		}
	};

	if (signal.type() ==  FsmSignalType::EnterState) {
		set_sprite(anim_state, state_index);
		arm(1.0f / animation.animation().fps());
	} else if (signal.type() == FsmSignalType::Alarm) {
		const auto state_sprite_count = animation.state(anim_state).sprites_size();
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
