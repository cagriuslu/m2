#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "HashMap.h"
#include "List.h"
#include "Vec2F.h"

// Forward declarations
typedef struct _Level Level;

typedef struct _PathfinderMap {
	HashMap blockedLocations;
} PathfinderMap;

int PathfinderMapInitFromLevel(PathfinderMap* pm, Level* level);
void PathfinderMapDeinit(PathfinderMap* pm);

int PathfinderMapFindGridSteps(PathfinderMap* pm, Vec2F from, Vec2F to, List* outReverseListOfVec2Is);
void PathfinderMapGridStepsToAnyAngle(List* listOfVec2Is, float shoulderWidth, List* outListOfVec2Is);

#endif
