#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "m2/Def.hh"
#include "m2/HashMap.hh"
#include "m2/List.hh"
#include <m2/vec2f.hh>

typedef struct _PathfinderMap {
	HashMap blockedLocations;
} PathfinderMap;

int PathfinderMap_Init(PathfinderMap* pm);
void PathfinderMap_Term(PathfinderMap* pm);

/// Returns XOK if outReverseListOfVec2Is has more than one points
/// Otherwise, returns XERR_PATH_NOT_FOUND.
M2Err PathfinderMap_FindPath(PathfinderMap* pm, m2::vec2f from, m2::vec2f to, List* outReverseListOfVec2Is);
M2Err _PathfinderMap_FindGridSteps(PathfinderMap* pm, m2::vec2f from, m2::vec2f to, List* outReverseListOfVec2Is);
void _PathfinderMap_GridStepsToAnyAngle(List* listOfVec2Is, List* outListOfVec2Is);


#endif