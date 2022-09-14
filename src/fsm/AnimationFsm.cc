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

m2::fsm::AnimationFsmData::AnimationFsmData(const AnimationFsmBlueprint* blueprint, GraphicId gfx_id) : blueprint(blueprint), gfx_id(gfx_id), state_sprite_idx(0, 0) {}

m2::FsmStateHandler m2::fsm::AnimationFsmData::state_func(Fsm<AnimationFsmData>& automaton, unsigned signal) {
	// Determine next sprite index
	auto blueprint = automaton.data.blueprint;
	auto curr_idx = automaton.data.state_sprite_idx;
	std::optional<decltype(curr_idx)> new_idx;
	if (signal == FSMSIG_ENTER) {
		new_idx = {0, 0};
		automaton.arm(1.0f / blueprint->frames_per_second);
	} else if (signal == FSMSIG_ALARM) {
		auto curr_state_sprite_count = blueprint->states[curr_idx.first].sprites.size();
		auto new_sprite_idx = (curr_idx.second + 1 == curr_state_sprite_count) ? 0 : curr_idx.second + 1;
		new_idx = {curr_idx.first, new_sprite_idx};
		automaton.arm(1.0f / blueprint->frames_per_second);
	} else if (FSMSIG_N <= signal) {
		auto signal_state_idx = signal - FSMSIG_N;
		if (signal_state_idx != curr_idx.first) {
			new_idx = {signal_state_idx, 0};
		}
	}

	// Change sprite if necessary
	if (new_idx) {
		auto gfx = GAME.graphics.get(automaton.data.gfx_id);
		if (gfx) {
			auto sprite_idx = blueprint->states[new_idx->first].sprites[new_idx->second];
			gfx->textureRect = m2g::sprites[sprite_idx].texture_rect;
			gfx->center_px = m2g::sprites[sprite_idx].obj_center_px;
		} else {
			LOG_WARN("Graphics component destroyed before Animation FSM");
		}
		automaton.data.state_sprite_idx = *new_idx;
	}

	return nullptr; // There is only one state :(
}

m2::fsm::AnimationFsm::AnimationFsm(const AnimationFsmBlueprint* blueprint, GraphicId gfx_id) : Fsm<AnimationFsmData>(AnimationFsmData{blueprint, gfx_id}) {}

void m2::fsm::AnimationFsm::set_state(unsigned state) {
	signal(FSMSIG_N + state);
}
