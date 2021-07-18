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

int PathfinderMap_InitFromLevel(PathfinderMap* pm, Level* level);
void PathfinderMap_Term(PathfinderMap* pm);

int PathfinderMap_FindPath(PathfinderMap* pm, Vec2F from, Vec2F to, List* outReverseListOfVec2Is);
int _PathfinderMap_FindGridSteps(PathfinderMap* pm, Vec2F from, Vec2F to, List* outReverseListOfVec2Is);
void _PathfinderMap_GridStepsToAnyAngle(List* listOfVec2Is, List* outListOfVec2Is);


#endif
