#include "m2/Pathfinder.hh"

#include <b2_fixture.h>

#include "m2/Box2DUtils.hh"
#include "m2/Component.hh"
#include "m2/Object.hh"
#include "m2/Game.hh"
#include <m2/vec2i.hh>
#include <stdbool.h>
#include <float.h>
#include <stdio.h>
#include <unordered_map>
#include <list>

#define ManhattanDistance(a, b) (abs((a).x - (b).x) + abs((a).y - (b).y))

int PathfinderMap_Init(PathfinderMap* pm) {
	pm->blocked_locations.clear();
    for (auto physique_it : GAME.physics) {
        ComponentPhysique* phy = physique_it.first;
		Object* obj = GAME.objects.get(phy->super.objId);
		if (obj && phy->body) {
			// TODO here it is assumed that bodies has one fixture
			b2Fixture* fixture = phy->body->GetFixtureList();
			const uint16_t categoryBits = fixture->GetFilterData().categoryBits;
			if (categoryBits & (CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF)) {
				// TODO here it is assumed that fixtures have one child
				b2AABB aabb = fixture->GetAABB(0);
				// AABB is bigger 0.01 meters than the object at each side
				// Decrease its size by 0.02 so that rounding can work
                b2AABB convervative_aabb = m2::box2d::aabb::expand(aabb, -0.02f);
				int lowerX = (int) roundf(convervative_aabb.lowerBound.x);
				int upperX = (int) roundf(convervative_aabb.upperBound.x);
				int lowerY = (int) roundf(convervative_aabb.lowerBound.y);
				int upperY = (int) roundf(convervative_aabb.upperBound.y);
				for (int y = lowerY; y <= upperY; y++) {
					for (int x = lowerX; x <= upperX; x++) {
						pm->blocked_locations.insert({x, y});
					}
				}
			}
		}
	}
	return 0;
}

void PathfinderMap_Term(PathfinderMap* pm) {
	pm->blocked_locations.clear();
}

M2Err PathfinderMap_FindPath(PathfinderMap* pm, m2::vec2f from, m2::vec2f to, std::list<m2::vec2i>& outReverseListOfVec2Is) {
	// Check if there is direct eyesight
	if (Box2DUtils_CheckEyeSight(from, to, CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF)) {
		auto fromI = m2::vec2i{from};
		auto toI = m2::vec2i{to};

        outReverseListOfVec2Is.clear();
		// Add `to` to list
        outReverseListOfVec2Is.push_back(toI);
		// Add `from` to list, if it is different than `to`
		if (fromI == toI) {
			return M2ERR_PATH_NOT_FOUND;
		} else {
            outReverseListOfVec2Is.push_back(fromI);
			return M2OK;
		}
	} else {
        std::list<m2::vec2i> grid_steps;

		M2Err anyAngleResult = M2ERR_PATH_NOT_FOUND;
		if (_PathfinderMap_FindGridSteps(pm, from, to, grid_steps) == M2OK) {
			_PathfinderMap_GridStepsToAnyAngle(grid_steps, outReverseListOfVec2Is);
			if (1 < outReverseListOfVec2Is.size()) {
				anyAngleResult = M2OK;
			}
		} else {
            outReverseListOfVec2Is.clear();
		}
		return anyAngleResult;
	}
}

struct PriorityListItem {
	float priority;
	m2::vec2i position;
};

M2Err _PathfinderMap_FindGridSteps(PathfinderMap* pm, m2::vec2f fromF, m2::vec2f toF, std::list<m2::vec2i>& outReverseListOfVec2Is) {
	auto from = m2::vec2i{ fromF };
	auto to = m2::vec2i{toF};

	PriorityListItem tmpPrioListItem;
	m2::vec2i tmpCameFrom;
	float tmpCostSoFar;

	// Holds the positions where will be explored next
    std::list<PriorityListItem> frontiers;
    frontiers.push_front({0.0f, from});

	// Holds from which position should you approach a certain position
	std::unordered_map<m2::vec2i, m2::vec2i, m2::vec2i_hash> came_from;
	came_from[from] = from;

	std::unordered_map<m2::vec2i, float, m2::vec2i_hash> cost_so_far;
	cost_so_far[from] = 0.0f;

	while (not frontiers.empty()) {
        auto curr_frontier_it = frontiers.begin();
        auto* frontierItem = &(*curr_frontier_it);

		// If next location to discover is the destination, stop
		if (frontierItem->position == to) {
			break;
		}

		// Find cost to current location
		const float costToCurrentFrontier = cost_so_far[frontierItem->position];

		// Gather neighbors of frontierItem
		m2::vec2i neighbors[4];
		float frontierToNeighborCosts[4];
		uint32_t neighborCount = 0;
		m2::vec2i topNeighbor = frontierItem->position + m2::vec2i(0, -1);
		if (not pm->blocked_locations.contains(topNeighbor) || (to == topNeighbor)) {
			neighbors[neighborCount] = topNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}
		m2::vec2i rightNeighbor = frontierItem->position + m2::vec2i(+1, 0);
		if (not pm->blocked_locations.contains(rightNeighbor) || (to == rightNeighbor)) {
			neighbors[neighborCount] = rightNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}
		m2::vec2i bottomNeighbor = frontierItem->position + m2::vec2i(0, +1);
		if (not pm->blocked_locations.contains(bottomNeighbor) || (to == bottomNeighbor)) {
			neighbors[neighborCount] = bottomNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}
		m2::vec2i leftNeighbor = frontierItem->position + m2::vec2i(-1, 0);
		if (not pm->blocked_locations.contains(leftNeighbor) || (to == leftNeighbor)) {
			neighbors[neighborCount] = leftNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}

		// Iterate over neighbors
		for (uint32_t neighIdx = 0; neighIdx < neighborCount; neighIdx++) {
			const auto neighbor = neighbors[neighIdx];
			const float frontierToNeighborCost = frontierToNeighborCosts[neighIdx];

			// Calculate the cost of traveling to neighbor from current location
			float newCostToNeighbor = costToCurrentFrontier + frontierToNeighborCost;
			// Find the previous cost of traveling to neighbor
			auto it = cost_so_far.find(neighbor);
			const float oldCostToNeighbor = (it != cost_so_far.end()) ? it->second : FLT_MAX;

			// If new path to neighbor is cheaper than the old path
			if (newCostToNeighbor < oldCostToNeighbor) {
				// Save new cost
				cost_so_far[neighbor] = newCostToNeighbor;
				// Calculate priority of neighbor with heuristic parameter (which is Manhattan distance to `to`)
				tmpPrioListItem.priority = newCostToNeighbor + ManhattanDistance(to, neighbor);
				tmpPrioListItem.position = neighbor;
				// Insert neighbor into frontiers
				bool inserted = false;
                for (auto frontier_it = frontiers.begin(); frontier_it != frontiers.end(); ++frontier_it) {
                    if (tmpPrioListItem.priority < frontier_it->priority) {
                        frontiers.insert(frontier_it, tmpPrioListItem);
                        inserted = true;
                        break;
                    }
                }
				if (not inserted) {
                    frontiers.push_back(tmpPrioListItem);
				}
				// Set the previous position of neighbor as the current position
				came_from[neighbor] = frontierItem->position;
			}
		}

		// Remove current position as we're done processing it
        frontiers.erase(curr_frontier_it);
	}

	M2Err result;
	// Check if there is a path
	auto it = came_from.find(to);
	if (it == came_from.end()) {
		result = M2ERR_PATH_NOT_FOUND;
	} else {
		result = M2OK;

        outReverseListOfVec2Is.clear();
		// Add `to` to list
        outReverseListOfVec2Is.push_back(to);
		// Built outReverseListOfVec2Is
		while (it != came_from.end() && from != it->second) {
			m2::vec2i data = it->second;
            outReverseListOfVec2Is.push_back(data);
			it = came_from.find(it->second);
		}
		// Add `from` to list as well
		if (it != came_from.end()) {
			m2::vec2i data = it->second;
            outReverseListOfVec2Is.push_back(data);
		}
	}
	return result;
}

void _PathfinderMap_GridStepsToAnyAngle(const std::list<m2::vec2i>& listOfVec2Is, std::list<m2::vec2i>& outListOfVec2Is) {
    outListOfVec2Is.clear();

	if (listOfVec2Is.size() < 2) {
		return;
	}
    auto point_1_it = listOfVec2Is.begin();
    outListOfVec2Is.push_back(*point_1_it);
    auto* point1 = &(*point_1_it);

	const m2::vec2i* prevPoint2 = nullptr;
    auto point_2_it = listOfVec2Is.begin();
	for (++point_2_it; point_2_it != listOfVec2Is.end(); ++point_2_it) {
        auto* point2 = &(*point_2_it);

		const bool eyeSight = Box2DUtils_CheckEyeSight(m2::vec2f{*point1}, m2::vec2f{*point2}, CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF);
        if (point_2_it == std::prev(listOfVec2Is.end(), 1)) {
            if (eyeSight) {
                // If we are processing the last point and there is an eye sight, add the last point
                outListOfVec2Is.push_back(*point2);
            } else {
                // If we are processing the last point and there is no eye sight, add previous and the last point
                if (prevPoint2) {
                    outListOfVec2Is.push_back(*prevPoint2);
                }
                outListOfVec2Is.push_back(*point2);
            }
        } else if (eyeSight) {
			// There is an eye sight, continue iterating
		} else {
			// There is no eye sight, add previous point
			if (prevPoint2) {
                outListOfVec2Is.push_back(*prevPoint2);
			}
			point1 = prevPoint2;
		}
		
		prevPoint2 = point2;
	}
}
