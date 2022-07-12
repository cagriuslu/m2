#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <m2/Vec2i.h>
#include <m2/Vec2f.h>
#include "Value.h"
#include <unordered_set>
#include <list>

typedef struct {
	std::unordered_set<m2::Vec2i, m2::Vec2iHash> blocked_locations;
} PathfinderMap;

int PathfinderMap_Init(PathfinderMap* pm);
void PathfinderMap_Term(PathfinderMap* pm);

/// Returns XOK if outReverseListOfVec2Is has more than one points
/// Otherwise, returns XERR_PATH_NOT_FOUND.

/// Returns reverse list of Vec2i's
m2::Value<std::list<m2::Vec2i>> PathfinderMap_FindPath(PathfinderMap* pm, m2::Vec2f from, m2::Vec2f to);

/// Returns reverse list of Vec2i's
m2::Value<std::list<m2::Vec2i>> _PathfinderMap_FindGridSteps(PathfinderMap* pm, m2::Vec2f from, m2::Vec2f to);

std::list<m2::Vec2i> _PathfinderMap_GridStepsToAnyAngle(const std::list<m2::Vec2i>& listOfVec2Is);

#endif
