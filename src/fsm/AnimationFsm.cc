#include <m2/fsm/AnimationFsm.h>
#include <m2/Game.h>

m2::AnimationFsm::AnimationFsm(m2g::pb::AnimationType animation_type, GraphicId gfx_id) : FsmBase({}), animation(GAME.animations[animation_type]), gfx_id(gfx_id), anim_state(GAME.animations[animation_type].animation().initial_state()), state_index(0) {
	init();
}

std::optional<m2::AnimationFsmState> m2::AnimationFsm::handle_signal(const AnimationFsmSignal& s) {
	auto set_sprite = [this](m2g::pb::AnimationStateType state, int index) {
		this->anim_state = state;
		this->state_index = index;
		auto gfx = LEVEL.graphics.get(this->gfx_id);
		if (gfx) {
			gfx->sprite = &GAME.sprites[animation.state(state).sprites(index)];
		} else {
			LOG_WARN("Graphics component destroyed before Animation FSM");
		}
	};

	if (s.type() ==  FsmSignalType::EnterState) {
		set_sprite(anim_state, state_index);
		arm(1.0f / animation.animation().fps());
	} else if (s.type() == FsmSignalType::Alarm) {
		auto state_sprite_count = animation.state(anim_state).sprites_size();
		set_sprite(anim_state, (state_index + 1) % state_sprite_count);
		arm(1.0f / animation.animation().fps());
	} else if (s.type() == FsmSignalType::Custom) {
		if (anim_state != s.animation_state_type()) {
			set_sprite(s.animation_state_type(), 0);
		}
	}
	return {};
}
