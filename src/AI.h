#ifndef AI_H
#define AI_H

#include "List.h"
#include "Stopwatch.h"
#include "Error.h"

typedef struct _AI {
	ListOfVec2I reversedWaypointList;
	Stopwatch waypointRecalculationStopwatch;
} AI;

XErr AI_Init(AI* ai);
void AI_Term(AI* ai);

#endif
