#include <m2/fsm/AnimationFsm.h>
#include <m2/Exception.h>

m2::fsm::AnimationFsmBlueprint m2::fsm::make_animation_fsm_blueprint(AnimationFsmBlueprint&& val) {
	// Assert that state indexes are zero indexed
	for (unsigned i = 0; i < val.states.size(); ++i) {
		if (val.states[i].state != i) {
			throw M2FATAL("State indexes are not zero indexed");
		}
	}
	return std::move(val);
}

m2::fsm::AnimationFsmBase::AnimationFsmBase(const AnimationFsmBlueprint* blueprint, GraphicId gfx_id) : blueprint(blueprint), gfx_id(gfx_id), state_sprite_idx(0, 0) {}

void* m2::fsm::AnimationFsmBase::state_func(Fsm<AnimationFsmBase>& fsm, int signal) {
	// Determine next sprite index
	auto blueprint = fsm.blueprint;
	auto curr_idx = fsm.state_sprite_idx;
	std::optional<decltype(curr_idx)> new_idx;
	if (signal == FSM_SIGNAL_ENTER) {
		new_idx = {0, 0};
		fsm.arm(1.0f / blueprint->frames_per_second);
	} else if (signal == FSM_SIGNAL_ALARM) {
		auto curr_state_sprite_count = blueprint->states[curr_idx.first].sprites.size();
		auto new_sprite_idx = (curr_idx.second + 1 == curr_state_sprite_count) ? 0 : curr_idx.second + 1;
		new_idx = {curr_idx.first, new_sprite_idx};
		fsm.arm(1.0f / blueprint->frames_per_second);
	} else if (FSM_SIGNAL_CUSTOM <= signal) {
		auto signal_state_idx = signal - FSM_SIGNAL_CUSTOM;
		if (signal_state_idx != curr_idx.first) {
			new_idx = {signal_state_idx, 0};
		}
	}

	// Change sprite if necessary
	if (new_idx) {
		auto gfx = GAME.graphics.get(fsm.gfx_id);
		if (gfx) {
			auto sprite_idx = blueprint->states[new_idx->first].sprites[new_idx->second];
			gfx->textureRect = m2g::sprites[sprite_idx].texture_rect;
			gfx->center_px = m2g::sprites[sprite_idx].obj_center_px;
		} else {
			LOG_WARN("Graphics component destroyed before Animation FSM");
		}
		fsm.state_sprite_idx = *new_idx;
	}

	return nullptr; // There is only one state :(
}
