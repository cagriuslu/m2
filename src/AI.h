#ifndef AI_H
#define AI_H

#include "List.h"

typedef struct _AI {
	List reversedVec2IWaypointList;
	unsigned waypointRecalculationStopwatch;
} AI;

int AIInit(AI* ai);
void AIDeinit(AI* ai);

#endif
