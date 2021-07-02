#ifndef AI_H
#define AI_H

#include "List.h"
#include "Stopwatch.h"
#include "Error.h"
#include "Vec2F.h"

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
	ListOfVec2I reversedWaypointList;
	Stopwatch waypointRecalculationStopwatch;
} AI;

XErr AI_Init(AI* ai);
void AI_Term(AI* ai);

#endif
