#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "Def.h"
#include "HashMap.h"
#include "List.h"
#include "Vec2F.h"

typedef struct _PathfinderMap {
	HashMap blockedLocations;
} PathfinderMap;

int PathfinderMap_Init(PathfinderMap* pm);
void PathfinderMap_Term(PathfinderMap* pm);

/// Returns XOK if outReverseListOfVec2Is has more than one points
/// Otherwise, returns XERR_PATH_NOT_FOUND.
M2Err PathfinderMap_FindPath(PathfinderMap* pm, Vec2F from, Vec2F to, List* outReverseListOfVec2Is);
M2Err _PathfinderMap_FindGridSteps(PathfinderMap* pm, Vec2F from, Vec2F to, List* outReverseListOfVec2Is);
void _PathfinderMap_GridStepsToAnyAngle(List* listOfVec2Is, List* outListOfVec2Is);


#endif
