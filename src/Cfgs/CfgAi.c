#include "../Cfg.h"

const CfgAi CFG_AI_CHASE_00 = {
	.behavior = CFG_AI_BEHAVIOR_CHASE,
	.capability = CFG_AI_CAPABILITY_MELEE,
	.triggerDistanceSquared_m = 25.0f,
	.attackDistanceSquared_m = 0.5625f,
	.giveUpDistanceSquared_m = 100.0f,
	.recalculationPeriod_s = 0.75f,
};
const CfgAi CFG_AI_KEEP_DISTANCE_00 = {
	.behavior = CFG_AI_BEHAVIOR_KEEP_DISTANCE,
	.capability = CFG_AI_CAPABILITY_RANGED,
	.triggerDistanceSquared_m = 25.0f,
	.attackDistanceSquared_m = 121.0f,
	.giveUpDistanceSquared_m = 225.0f,
	.recalculationPeriod_s = 0.8f,
	.keepDistanceDistanceSquared_m = 156.25f
};
const CfgAi CFG_AI_HIT_N_RUN_00 = {
	.behavior = CFG_AI_BEHAVIOR_HIT_N_RUN,
	.capability = CFG_AI_CAPABILITY_MELEE,
	.triggerDistanceSquared_m = 25.0f,
	.attackDistanceSquared_m = 0.5625f,
	.giveUpDistanceSquared_m = 225.0f,
	.recalculationPeriod_s = 0.8f,
	.hitNRunHitDistanceSquared_m = 0.25f,
	.hitNRunHitDuration_s = 2.0f,
	.hitNRunRunDistanceSquared_m = 100.0f,
	.hitNRunRunDuration_s = 4.0f
};
