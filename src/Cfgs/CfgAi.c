#include "../Cfg.h"

const CfgAi CFG_AI_CHASE_00 = {
	.behavior = CFG_AI_BEHAVIOR_CHASE,
	.capability = CFG_AI_CAPABILITY_MELEE,
	.triggerDistance_m = 5.0f,
	.attackDistance_m = 0.75f,
	.giveUpDistance_m = 10.0f,
	.recalculationPeriod_s = 0.8f,
};
const CfgAi CFG_AI_KEEP_DISTANCE_00 = {
	.behavior = CFG_AI_BEHAVIOR_KEEP_DISTANCE,
	.capability = CFG_AI_CAPABILITY_RANGED,
	.triggerDistance_m = 5.0f,
	.attackDistance_m = 11.0f,
	.giveUpDistance_m = 15.0f,
	.recalculationPeriod_s = 0.8f,
	.keepDistanceDistance_m = 12.5f
};
const CfgAi CFG_AI_HIT_N_RUN_00 = {
	.behavior = CFG_AI_BEHAVIOR_HIT_N_RUN,
	.capability = CFG_AI_CAPABILITY_MELEE,
	.triggerDistance_m = 5.0f,
	.attackDistance_m = 0.75f,
	.giveUpDistance_m = 15.0f,
	.recalculationPeriod_s = 0.8f,
	.hitNRunHitDistance_m = 0.5f,
	.hitNRunHitDuration_s = 2.0f,
	.hitNRunRunDistance_m = 10.0f,
	.hitNRunRunDuration_s = 4.0f
};
