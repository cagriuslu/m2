#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "m2/Def.h"
#include <m2/Vec2i.hh>
#include <m2/Vec2f.h>
#include <unordered_set>
#include <list>

typedef struct {
	std::unordered_set<m2::Vec2i, m2::Vec2iHash> blocked_locations;
} PathfinderMap;

int PathfinderMap_Init(PathfinderMap* pm);
void PathfinderMap_Term(PathfinderMap* pm);

/// Returns XOK if outReverseListOfVec2Is has more than one points
/// Otherwise, returns XERR_PATH_NOT_FOUND.
M2Err PathfinderMap_FindPath(PathfinderMap* pm, m2::Vec2f from, m2::Vec2f to, std::list<m2::Vec2i>& outReverseListOfVec2Is);
M2Err _PathfinderMap_FindGridSteps(PathfinderMap* pm, m2::Vec2f from, m2::Vec2f to, std::list<m2::Vec2i>& outReverseListOfVec2Is);
void _PathfinderMap_GridStepsToAnyAngle(const std::list<m2::Vec2i>& listOfVec2Is, std::list<m2::Vec2i>& outListOfVec2Is);


#endif
