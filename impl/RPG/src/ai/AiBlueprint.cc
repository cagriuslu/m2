#include <impl/private/ai/AiBlueprint.h>

std::unique_ptr<impl::ai::AiState> impl::ai::AiBlueprint::get_state() const {
	using namespace type;

	std::unique_ptr<AiState> state;
	if (std::holds_alternative<ChaseBlueprint>(variant)) {
		state = std::make_unique<ChaseState>(this);
	} else if (std::holds_alternative<HitNRunBlueprint>(variant)) {
		state = std::make_unique<HitNRunState>(this);
	} else if (std::holds_alternative<KeepDistanceBlueprint>(variant)) {
		state = std::make_unique<KeepDistanceState>(this);
	} else if (std::holds_alternative<PatrolBlueprint>(variant)) {
		state = std::make_unique<PatrolState>(this);
	}

	return state;
}

const impl::ai::AiBlueprint impl::ai::blueprint::chase_000 = {
		.capability = CAPABILITY_MELEE,
		.trigger_distance_squared_m = 25.0f,
		.attack_distance_squared_m = 0.5625f,
		.give_up_distance_squared_m = 100.0f,
		.recalculation_period_s = 0.75f,
		.variant = type::ChaseBlueprint{}
};
