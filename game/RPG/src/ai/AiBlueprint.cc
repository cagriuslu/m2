#include <rpg/ai/AiBlueprint.h>

const ai::AiBlueprint ai::chase_000 = {
		.capability = CAPABILITY_MELEE,
		.trigger_distance_m = 5.0f,
		.attack_distance_m = 0.75f,
		.give_up_distance_m = INFINITY,
		.recalculation_period_s = 0.75f,
		.variant = type::ChaseBlueprint{}
};
