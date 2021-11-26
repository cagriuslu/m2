#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "HashMap.h"
#include "List.h"
#include "Vec2F.h"

typedef struct _PathfinderMap {
	HashMap blockedLocations;
} PathfinderMap;

int PathfinderMap_Init(PathfinderMap* pm);
void PathfinderMap_Term(PathfinderMap* pm);

int PathfinderMap_FindPath(PathfinderMap* pm, Vec2F from, Vec2F to, List* outReverseListOfVec2Is);
int _PathfinderMap_FindGridSteps(PathfinderMap* pm, Vec2F from, Vec2F to, List* outReverseListOfVec2Is);
void _PathfinderMap_GridStepsToAnyAngle(List* listOfVec2Is, List* outListOfVec2Is);


#endif
