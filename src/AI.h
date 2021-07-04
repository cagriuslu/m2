#ifndef AI_H
#define AI_H

#include "List.h"
#include "Stopwatch.h"
#include "Error.h"
#include "Vec2F.h"

#define AI_CAPABILITY_ATTACK_PLAYER_RANGED (1 << 0)
#define AI_CAPABILITY_ATTACK_PLAYER_MELEE  (1 << 1)
#define AI_CAPABILITY_GIVE_UP_ON_PLAYER    (1 << 2)
#define AI_CAPABILITY_WANDER_AROUND_HOME   (1 << 3)
#define AI_CAPABILITY_PATROL_AREA          (1 << 4)
#define AI_CAPABILITY_PATROL_WAYPOINTS     (1 << 5)
#define AI_CAPABILITY_HARASS_PLAYER_RANGED (1 << 6) // hit and run
#define AI_CAPABILITY_HARASS_PLAYER_MELEE  (1 << 7) // hit and run

typedef enum _AIMode {
	AI_IDLE = 0,
	AI_GOING_AFTER_PLAYER,
	AI_GOING_BACK_TO_HOME,
} AIMode;

typedef struct _AI {
	AIMode mode;
	unsigned recalculationPeriod;
	Vec2F homePosition;
	float triggerDistance;
	ListOfVec2Is reversedWaypointList;
	Stopwatch waypointRecalculationStopwatch;
} AI;

XErr AI_Init(AI* ai);
void AI_Term(AI* ai);

#endif
