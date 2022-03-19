#include "m2/Pathfinder.hh"
#include "m2/List.hh"
#include "m2/Box2DUtils.hh"
#include "m2/Component.hh"
#include "m2/Object.hh"
#include "m2/Game.hh"
#include <m2/vec2i.hh>
#include <stdbool.h>
#include <float.h>
#include <stdio.h>
#include <unordered_map>

#define ManhattanDistance(a, b) (abs((a).x - (b).x) + abs((a).y - (b).y))

int PathfinderMap_Init(PathfinderMap* pm) {
	*pm = {};

	for (ComponentPhysique* phy = static_cast<ComponentPhysique *>(Pool_GetFirst(
			&GAME->physics)); phy; phy = static_cast<ComponentPhysique *>(Pool_GetNext(&GAME->physics, phy))) {
		Object* obj = static_cast<Object *>(Pool_GetById(&GAME->objects, phy->super.objId));
		if (obj && phy->body) {
			const int fixtureCount = Box2DBodyGetFixtureCount(phy->body);
			for (int i = 0; i < fixtureCount; i++) {
				Box2DFixture* fixture = Box2DBodyGetFixture(phy->body, i);
				const uint16_t categoryBits = Box2DFixtureGetCategory(fixture);
				if (categoryBits & (CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF)) {
					const int proxyCount = Box2DFixtureGetProxyCount(fixture);
					for (int proxyIdx = 0; proxyIdx < proxyCount; proxyIdx++) {
						AABB aabb = Box2DFixtureGetAABB(fixture, proxyIdx);
						// AABB is bigger 0.01 meters than the object at each side
						// Decrease its size by 0.02 so that rounding can work						
						AABB conservativeAabb = AABB{aabb.lowerBound + m2::vec2f{ 0.02f, 0.02f}, aabb.upperBound - m2::vec2f{ 0.02f, 0.02f}};
						int lowerX = (int) roundf(conservativeAabb.lowerBound.x);
						int upperX = (int) roundf(conservativeAabb.upperBound.x);
						int lowerY = (int) roundf(conservativeAabb.lowerBound.y);
						int upperY = (int) roundf(conservativeAabb.upperBound.y);
						for (int y = lowerY; y <= upperY; y++) {
							for (int x = lowerX; x <= upperX; x++) {
								pm->blocked_locations.insert({x, y});
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

void PathfinderMap_Term(PathfinderMap* pm) {
	*pm = {};
}

M2Err PathfinderMap_FindPath(PathfinderMap* pm, m2::vec2f from, m2::vec2f to, List* outReverseListOfVec2Is) {
	// Check if there is direct eyesight
	if (Box2DUtils_CheckEyeSight(from, to, CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF)) {
		auto fromI = m2::vec2i{from};
		auto toI = m2::vec2i{to};

		List_Clear(outReverseListOfVec2Is);
		// Add `to` to list
		List_Append(outReverseListOfVec2Is, &toI, NULL);
		// Add `from` to list, if it is different than `to`
		if (fromI == toI) {
			return M2ERR_PATH_NOT_FOUND;
		} else {
			List_Append(outReverseListOfVec2Is, &fromI, NULL);
			return M2OK;
		}
	} else {
		List gridSteps;
		List_Init(&gridSteps, 10, sizeof(m2::vec2i));

		M2Err anyAngleResult = M2ERR_PATH_NOT_FOUND;
		if (_PathfinderMap_FindGridSteps(pm, from, to, &gridSteps) == M2OK) {
			_PathfinderMap_GridStepsToAnyAngle(&gridSteps, outReverseListOfVec2Is);
			if (1 < List_Length(outReverseListOfVec2Is)) {
				anyAngleResult = M2OK;
			}
		} else {
			List_Clear(outReverseListOfVec2Is);
		}

		List_Term(&gridSteps);
		return anyAngleResult;
	}
}

typedef struct _PriorityListItem {
	float priority;
	m2::vec2i position;
} PriorityListItem;

M2Err _PathfinderMap_FindGridSteps(PathfinderMap* pm, m2::vec2f fromF, m2::vec2f toF, List* outReverseListOfVec2Is) {
	auto from = m2::vec2i{ fromF };
	auto to = m2::vec2i{toF};

	PriorityListItem tmpPrioListItem;
	m2::vec2i tmpCameFrom;
	float tmpCostSoFar;

	// Holds the positions where will be explored next
	List frontiers;
	List_Init(&frontiers, 10, sizeof(PriorityListItem));
	tmpPrioListItem = PriorityListItem{0.0f, from};
	List_Prepend(&frontiers, &tmpPrioListItem, NULL);

	// Holds from which position should you approach a certain position
	std::unordered_map<m2::vec2i, m2::vec2i, m2::vec2i_hash> came_from;
	came_from[from] = from;

	std::unordered_map<m2::vec2i, float, m2::vec2i_hash> cost_so_far;
	cost_so_far[from] = 0.0f;

	while (0 < frontiers.bucket.size) {
		const ID frontierIterator = List_GetFirst(&frontiers);
		PriorityListItem* frontierItem = static_cast<PriorityListItem *>(List_GetData(&frontiers, frontierIterator));

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
				for (ID priorityCheckIterator = List_GetFirst(&frontiers); priorityCheckIterator; priorityCheckIterator = List_GetNext(&frontiers, priorityCheckIterator)) {
					PriorityListItem* item = static_cast<PriorityListItem *>(List_GetData(&frontiers,
																						  priorityCheckIterator));
					if (tmpPrioListItem.priority < item->priority) {
						List_InsertBefore(&frontiers, priorityCheckIterator, &tmpPrioListItem, NULL);
						inserted = true;
						break;
					}
				}
				if (!inserted) {
					List_Append(&frontiers, &tmpPrioListItem, NULL);
				}
				// Set the previous position of neighbor as the current position
				came_from[neighbor] = frontierItem->position;
			}
		}

		// Remove current position as we're done processing it
		List_Remove(&frontiers, frontierIterator);
	}

	M2Err result;
	// Check if there is a path
	auto it = came_from.find(to);
	if (it == came_from.end()) {
		result = M2ERR_PATH_NOT_FOUND;
	} else {
		result = M2OK;

		List_Clear(outReverseListOfVec2Is);
		// Add `to` to list
		List_Append(outReverseListOfVec2Is, &to, NULL);
		// Built outReverseListOfVec2Is
		while (it != came_from.end() && from != it->second) {
			m2::vec2i data = it->second;
			List_Append(outReverseListOfVec2Is, &data, NULL);
			it = came_from.find(it->second);
		}
		// Add `from` to list as well
		if (it != came_from.end()) {
			m2::vec2i data = it->second;
			List_Append(outReverseListOfVec2Is, &data, NULL);
		}
	}

	// Cleanup
	List_Term(&frontiers);
	return result;
}

void _PathfinderMap_GridStepsToAnyAngle(List* listOfVec2Is, List* outListOfVec2Is) {
	List_Clear(outListOfVec2Is);

	const ID point1Iterator = List_GetFirst(listOfVec2Is);
	auto point1 = static_cast<m2::vec2i*>(List_GetData(listOfVec2Is, point1Iterator));
	// Add first point
	if (point1) {
		List_Append(outListOfVec2Is, point1, NULL);
	}
	
	if (point1 == NULL || listOfVec2Is->bucket.size < 2) {
		return;
	}

	m2::vec2i* prevPoint2 = nullptr;
	for (ID point2Iterator = List_GetNext(listOfVec2Is, point1Iterator); point2Iterator; point2Iterator = List_GetNext(listOfVec2Is, point2Iterator)) {
		auto point2 = static_cast<m2::vec2i*>(List_GetData(listOfVec2Is, point2Iterator));

		const bool eyeSight = Box2DUtils_CheckEyeSight(m2::vec2f{*point1}, m2::vec2f{*point2}, CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF);
		if (List_GetLast(listOfVec2Is) == point2Iterator && eyeSight) {
			// If we are processing the last point and there is an eye sight, add the last point
			List_Append(outListOfVec2Is, point2, NULL);
		} else if (List_GetLast(listOfVec2Is) == point2Iterator && !eyeSight) {
			// If we are processing the last point and there is no eye sight, add previous and the last point
			if (prevPoint2) {
				List_Append(outListOfVec2Is, prevPoint2, NULL);
			}
			List_Append(outListOfVec2Is, point2, NULL);
		} else if (eyeSight) {
			// There is an eye sight, continue iterating
		} else {
			// There is no eye sight, add previous point
			if (prevPoint2) {
				List_Append(outListOfVec2Is, prevPoint2, NULL);
			}
			point1 = prevPoint2;
		}
		
		prevPoint2 = point2;
	}
}
