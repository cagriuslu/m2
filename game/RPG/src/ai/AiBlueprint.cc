#include <rpg/ai/AiBlueprint.h>

const ai::AiBlueprint ai::chase_000 = {
		.capability = CAPABILITY_MELEE,
		.trigger_distance_squared_m = 25.0f,
		.attack_distance_squared_m = 0.5625f,
		.give_up_distance_squared_m = 100.0f,
		.recalculation_period_s = 0.75f,
		.variant = type::ChaseBlueprint{}
};
