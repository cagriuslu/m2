#ifndef AI_H
#define AI_H

#include "Array.h"
#include "List.h"
#include "Stopwatch.h"
#include "Def.h"
#include "Vec2F.h"

#define AI_CAPABILITY_RANGED_ATTACK        (1 << 0) // Ranged
#define AI_CAPABILITY_MELEE_ATTACK         (1 << 1) // Melee
#define AI_CAPABILITY_WANDER_AROUND_HOME   (1 << 2) // WanderHome
#define AI_CAPABILITY_PATROL_AREA          (1 << 3) // PatrolArea
#define AI_CAPABILITY_PATROL_WAYPOINTS     (1 << 4) // PatrolPoints
#define AI_CAPABILITY_HARASS               (1 << 5) // Harass

typedef enum _AIMode {
	AI_IDLE = 0,
	AI_GOING_AFTER_PLAYER,
	AI_GOING_BACK_TO_HOME,
} AIMode;

typedef struct _AI {
	uint32_t capabilities;
	
	AIMode mode;
	unsigned recalculationPeriod;
	Stopwatch recalculationStopwatch;
	unsigned attackPeriod;
	Stopwatch attackStopwatch;
	
	Vec2F homePosition;
	// If there is only one point, it is the home position
	// If there are more than one points, they are either waypoints,
	// or corners of a concave area.
	ArrayOfVec2Fs interestPoints;
	float triggerDistance;
	
	ListOfVec2Is reversedWaypointList;
} AI;

XErr AI_Init(AI* ai);
void AI_Term(AI* ai);

#endif
