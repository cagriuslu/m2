#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "m2/Def.hh"
#include <m2/vec2i.hh>
#include <m2/vec2f.hh>
#include <unordered_set>

typedef struct {
	std::unordered_set<m2::vec2i, m2::vec2i_hash> blocked_locations;
} PathfinderMap;

int PathfinderMap_Init(PathfinderMap* pm);
void PathfinderMap_Term(PathfinderMap* pm);

/// Returns XOK if outReverseListOfVec2Is has more than one points
/// Otherwise, returns XERR_PATH_NOT_FOUND.
M2Err PathfinderMap_FindPath(PathfinderMap* pm, m2::vec2f from, m2::vec2f to, std::list<m2::vec2i>& outReverseListOfVec2Is);
M2Err _PathfinderMap_FindGridSteps(PathfinderMap* pm, m2::vec2f from, m2::vec2f to, std::list<m2::vec2i>& outReverseListOfVec2Is);
void _PathfinderMap_GridStepsToAnyAngle(const std::list<m2::vec2i>& listOfVec2Is, std::list<m2::vec2i>& outListOfVec2Is);


#endif
