#include "Pathfinder.h"
#include "List.h"
#include "Box2DUtils.h"
#include "Component.h"
#include "Object.h"
#include "Game.h"
#include <string.h>
#include <stdbool.h>
#include <float.h>
#include <stdio.h>

#define XYToHashMapKey(x, y) (((int64_t)(y) << 32) | ((int64_t)(x) & 0x00000000FFFFFFFF))
#define Vec2IToHashMapKey(vec2i) XYToHashMapKey((vec2i).x, (vec2i).y)
#define ManhattanDistance(a, b) (abs((a).x - (b).x) + abs((a).y - (b).y))

int PathfinderMap_Init(PathfinderMap* pm) {
	memset(pm, 0, sizeof(PathfinderMap));
	REFLECT_ERROR(HashMap_Init(&pm->blockedLocations, sizeof(bool), NULL));

	for (ComponentPhysics* phy = Pool_GetFirst(&GAME->physics); phy; phy = Pool_GetNext(&GAME->physics, phy)) {
		Object* obj = Pool_GetById(&GAME->objects, phy->super.objId);
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
						AABB conservativeAabb = (AABB){Vec2F_Add(aabb.lowerBound, (Vec2F){ 0.02f, 0.02f}), Vec2F_Sub(aabb.upperBound, (Vec2F){ 0.02f, 0.02f})};
						int lowerX = (int) roundf(conservativeAabb.lowerBound.x);
						int upperX = (int) roundf(conservativeAabb.upperBound.x);
						int lowerY = (int) roundf(conservativeAabb.lowerBound.y);
						int upperY = (int) roundf(conservativeAabb.upperBound.y);
						for (int y = lowerY; y <= upperY; y++) {
							for (int x = lowerX; x <= upperX; x++) {
								bool blocked = true;
								HashMap_SetInt64Key(&pm->blockedLocations, XYToHashMapKey(x, y), &blocked); // TODO check result
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
	HashMap_Term(&pm->blockedLocations);
	memset(pm, 0, sizeof(PathfinderMap));
}

int PathfinderMap_FindPath(PathfinderMap* pm, Vec2F from, Vec2F to, List* outReverseListOfVec2Is) {
	List gridSteps;
	List_Init(&gridSteps, sizeof(Vec2I));
	const int pathfinderResult = _PathfinderMap_FindGridSteps(pm, from, to, &gridSteps);
	if (pathfinderResult == 0) {
		_PathfinderMap_GridStepsToAnyAngle(&gridSteps, outReverseListOfVec2Is);
	} else {
		List_Clear(outReverseListOfVec2Is);
	}
	List_Term(&gridSteps);
	return pathfinderResult;
}

typedef struct _PriorityListItem {
	float priority;
	Vec2I position;
} PriorityListItem;

int _PathfinderMap_FindGridSteps(PathfinderMap* pm, Vec2F fromF, Vec2F toF, List* outReverseListOfVec2Is) {
	Vec2I from = Vec2I_From2F(fromF);
	Vec2I to = Vec2I_From2F(toF);

	PriorityListItem tmpPrioListItem;
	Vec2I tmpCameFrom;
	float tmpCostSoFar;

	// Holds the positions where will be explored next
	List frontiers;
	List_Init(&frontiers, sizeof(PriorityListItem));
	tmpPrioListItem = (PriorityListItem){0.0f, from};
	List_Prepend(&frontiers, &tmpPrioListItem, NULL);

	// Holds from which position should you approach a certain position
	HashMap cameFrom;
	HashMap_Init(&cameFrom, sizeof(Vec2I), NULL);
	tmpCameFrom = from;
	HashMap_SetInt64Key(&cameFrom, Vec2IToHashMapKey(from), &tmpCameFrom);

	HashMap costSoFar;
	HashMap_Init(&costSoFar, sizeof(float), NULL);
	tmpCostSoFar = 0.0f;
	HashMap_SetInt64Key(&costSoFar, Vec2IToHashMapKey(from), &tmpCostSoFar);

	while (0 < frontiers.bucket.size) {
		const ID frontierIterator = List_GetFirst(&frontiers);
		PriorityListItem* frontierItem = List_GetData(&frontiers, frontierIterator);

		// If next location to discover is the destination, stop
		if (Vec2I_Equals(frontierItem->position, to)) {
			break;
		}

		// Find cost to current location
		float* costToCurrentFrontierPtr = HashMap_GetInt64Key(&costSoFar, Vec2IToHashMapKey(frontierItem->position));
		const float costToCurrentFrontier = *costToCurrentFrontierPtr;

		// Gather neighbors of frontierItem
		Vec2I neighbors[4];
		float frontierToNeighborCosts[4];
		uint32_t neighborCount = 0;
		Vec2I topNeighbor = Vec2I_Add(frontierItem->position, (Vec2I) {0, -1});
		if (HashMap_GetInt64Key(&pm->blockedLocations, Vec2IToHashMapKey(topNeighbor)) == NULL || Vec2I_Equals(to, topNeighbor)) {
			neighbors[neighborCount] = topNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}
		Vec2I rightNeighbor = Vec2I_Add(frontierItem->position, (Vec2I) { +1, 0 });
		if (HashMap_GetInt64Key(&pm->blockedLocations, Vec2IToHashMapKey(rightNeighbor)) == NULL || Vec2I_Equals(to, rightNeighbor)) {
			neighbors[neighborCount] = rightNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}
		Vec2I bottomNeighbor = Vec2I_Add(frontierItem->position, (Vec2I) { 0, +1 });
		if (HashMap_GetInt64Key(&pm->blockedLocations, Vec2IToHashMapKey(bottomNeighbor)) == NULL || Vec2I_Equals(to, bottomNeighbor)) {
			neighbors[neighborCount] = bottomNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}
		Vec2I leftNeighbor = Vec2I_Add(frontierItem->position, (Vec2I) { -1, 0 });
		if (HashMap_GetInt64Key(&pm->blockedLocations, Vec2IToHashMapKey(leftNeighbor)) == NULL || Vec2I_Equals(to, leftNeighbor)) {
			neighbors[neighborCount] = leftNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}

		// Iterate over neighbors
		for (uint32_t neighIdx = 0; neighIdx < neighborCount; neighIdx++) {
			const Vec2I neighbor = neighbors[neighIdx];
			const float frontierToNeighborCost = frontierToNeighborCosts[neighIdx];

			// Calculate the cost of traveling to neighbor from current location
			float newCostToNeighbor = costToCurrentFrontier + frontierToNeighborCost;
			// Find the previous cost of traveling to neighbor
			float* oldCostToNeighborPtr = HashMap_GetInt64Key(&costSoFar, Vec2IToHashMapKey(neighbor));
			const float oldCostToNeighbor = oldCostToNeighborPtr ? *oldCostToNeighborPtr : FLT_MAX;

			// If new path to neighbor is cheaper than the old path
			if (newCostToNeighbor < oldCostToNeighbor) {
				// Save new cost
				HashMap_SetInt64Key(&costSoFar, Vec2IToHashMapKey(neighbor), &newCostToNeighbor);
				// Calculate priority of neighbor with heuristic parameter (which is Manhattan distance to `to`)
				tmpPrioListItem.priority = newCostToNeighbor + ManhattanDistance(to, neighbor);
				tmpPrioListItem.position = neighbor;
				// Insert neighbor into frontiers
				bool inserted = false;
				for (ID priorityCheckIterator = List_GetFirst(&frontiers); priorityCheckIterator; priorityCheckIterator = List_GetNext(&frontiers, priorityCheckIterator)) {
					PriorityListItem* item = List_GetData(&frontiers, priorityCheckIterator);
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
				tmpCameFrom = frontierItem->position;
				HashMap_SetInt64Key(&cameFrom, Vec2IToHashMapKey(neighbor), &tmpCameFrom);
			}
		}

		// Remove current position as we're done processing it
		List_Remove(&frontiers, frontierIterator);
	}

	int result;
	// Check if there is a path
	Vec2I* currentCameFrom = HashMap_GetInt64Key(&cameFrom, Vec2IToHashMapKey(to));
	if (currentCameFrom == NULL) {
		result = XERR_PATH_NOT_FOUND;
	} else {
		result = 0;

		List_Clear(outReverseListOfVec2Is);
		// Add `to` to list
		List_Append(outReverseListOfVec2Is, &to, NULL);
		// Built outReverseListOfVec2Is
		while (currentCameFrom && Vec2I_Equals(from, *currentCameFrom) == false) {
			List_Append(outReverseListOfVec2Is, currentCameFrom, NULL);
			currentCameFrom = HashMap_GetInt64Key(&cameFrom, Vec2IToHashMapKey(*currentCameFrom));
		}
		// Add `from` to list as well
		if (currentCameFrom) {
			List_Append(outReverseListOfVec2Is, currentCameFrom, NULL);
		}
	}

	// Cleanup
	HashMap_Term(&costSoFar);
	HashMap_Term(&cameFrom);
	List_Term(&frontiers);
	return result;
}

void _PathfinderMap_GridStepsToAnyAngle(List* listOfVec2Is, List* outListOfVec2Is) {
	List_Clear(outListOfVec2Is);

	const ID point1Iterator = List_GetFirst(listOfVec2Is);
	Vec2I* point1 = List_GetData(listOfVec2Is, point1Iterator);
	// Add first point
	if (point1) {
		List_Append(outListOfVec2Is, point1, NULL);
	}
	
	if (point1 == NULL || listOfVec2Is->bucket.size < 2) {
		return;
	}

	Vec2I* prevPoint2 = NULL;
	for (ID point2Iterator = List_GetNext(listOfVec2Is, point1Iterator); point2Iterator; point2Iterator = List_GetNext(listOfVec2Is, point2Iterator)) {
		Vec2I* point2 = List_GetData(listOfVec2Is, point2Iterator);

		const bool eyeSight = Box2DUtils_CheckEyeSight(Vec2F_FromVec2I(*point1), Vec2F_FromVec2I(*point2), CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF);
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
