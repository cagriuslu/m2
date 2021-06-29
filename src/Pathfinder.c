#include "Pathfinder.h"
#include "List.h"
#include "Box2DUtils.h"
#include "Component.h"
#include "Object.h"
#include "Level.h"
#include <string.h>
#include <stdbool.h>
#include <float.h>
#include <stdio.h>

#define XYToHashMapKey(x, y) (((int64_t)(y) << 32) | ((int64_t)(x) & 0x00000000FFFFFFFF))
#define Vec2IToHashMapKey(vec2i) XYToHashMapKey((vec2i).x, (vec2i).y)
#define ManhattanDistance(a, b) (abs((a).x - (b).x) + abs((a).y - (b).y))

int PathfinderMapInitFromLevel(PathfinderMap* pm, Level* level) {
	memset(pm, 0, sizeof(PathfinderMap));
	PROPAGATE_ERROR(HashMapInit(&pm->blockedLocations, sizeof(bool)));

	for (PhysicsComponent* phy = Bucket_GetFirst(&level->physics); phy; phy = Bucket_GetNext(&level->physics, phy)) {
		Object* obj = Bucket_GetById(&level->objects, phy->super.objId);
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
						AABB conservativeAabb = (AABB){Vec2FAdd(aabb.lowerBound, (Vec2F){ 0.02f, 0.02f}), Vec2FSub(aabb.upperBound, (Vec2F){ 0.02f, 0.02f})};
						int lowerX = (int) roundf(conservativeAabb.lowerBound.x);
						int upperX = (int) roundf(conservativeAabb.upperBound.x);
						int lowerY = (int) roundf(conservativeAabb.lowerBound.y);
						int upperY = (int) roundf(conservativeAabb.upperBound.y);
						for (int y = lowerY; y <= upperY; y++) {
							for (int x = lowerX; x <= upperX; x++) {
								bool blocked = true;
								HashMapSetIntKey(&pm->blockedLocations, XYToHashMapKey(x, y), &blocked); // TODO check result
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

void PathfinderMapDeinit(PathfinderMap* pm) {
	HashMapDeinit(&pm->blockedLocations);
	memset(pm, 0, sizeof(PathfinderMap));
}

typedef struct _PriorityListItem {
	float priority;
	Vec2I position;
} PriorityListItem;

int PathfinderMapFindGridSteps(PathfinderMap* pm, Vec2F fromF, Vec2F toF, List* outReverseListOfVec2Is) {
	Vec2I from = Vec2FTo2I(fromF);
	Vec2I to = Vec2FTo2I(toF);

	PriorityListItem tmpPrioListItem;
	Vec2I tmpCameFrom;
	float tmpCostSoFar;

	// Holds the positions where will be explored next
	List frontiers;
	ListInit(&frontiers, sizeof(PriorityListItem));
	tmpPrioListItem = (PriorityListItem){0.0f, from};
	ListPrepend(&frontiers, &tmpPrioListItem, NULL);

	// Holds from which position should you approach a certain position
	HashMap cameFrom;
	HashMapInit(&cameFrom, sizeof(Vec2I));
	tmpCameFrom = from;
	HashMapSetIntKey(&cameFrom, Vec2IToHashMapKey(from), &tmpCameFrom);

	HashMap costSoFar;
	HashMapInit(&costSoFar, sizeof(float));
	tmpCostSoFar = 0.0f;
	HashMapSetIntKey(&costSoFar, Vec2IToHashMapKey(from), &tmpCostSoFar);

	while (0 < frontiers.bucket.size) {
		const ID frontierIterator = ListGetFirst(&frontiers);
		PriorityListItem* frontierItem = ListGetData(&frontiers, frontierIterator);

		// If next location to discover is the destination, stop
		if (Vec2IEquals(frontierItem->position, to)) {
			break;
		}

		// Find cost to current location
		float* costToCurrentFrontierPtr = HashMapGetIntKey(&costSoFar, Vec2IToHashMapKey(frontierItem->position));
		const float costToCurrentFrontier = *costToCurrentFrontierPtr;

		// Gather neighbors of frontierItem
		Vec2I neighbors[4];
		float frontierToNeighborCosts[4];
		uint32_t neighborCount = 0;
		Vec2I topNeighbor = Vec2IAdd(frontierItem->position, (Vec2I) {0, -1});
		if (HashMapGetIntKey(&pm->blockedLocations, Vec2IToHashMapKey(topNeighbor)) == NULL || Vec2IEquals(to, topNeighbor)) {
			neighbors[neighborCount] = topNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}
		Vec2I rightNeighbor = Vec2IAdd(frontierItem->position, (Vec2I) { +1, 0 });
		if (HashMapGetIntKey(&pm->blockedLocations, Vec2IToHashMapKey(rightNeighbor)) == NULL || Vec2IEquals(to, rightNeighbor)) {
			neighbors[neighborCount] = rightNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}
		Vec2I bottomNeighbor = Vec2IAdd(frontierItem->position, (Vec2I) { 0, +1 });
		if (HashMapGetIntKey(&pm->blockedLocations, Vec2IToHashMapKey(bottomNeighbor)) == NULL || Vec2IEquals(to, bottomNeighbor)) {
			neighbors[neighborCount] = bottomNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}
		Vec2I leftNeighbor = Vec2IAdd(frontierItem->position, (Vec2I) { -1, 0 });
		if (HashMapGetIntKey(&pm->blockedLocations, Vec2IToHashMapKey(leftNeighbor)) == NULL || Vec2IEquals(to, leftNeighbor)) {
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
			float* oldCostToNeighborPtr = HashMapGetIntKey(&costSoFar, Vec2IToHashMapKey(neighbor));
			const float oldCostToNeighbor = oldCostToNeighborPtr ? *oldCostToNeighborPtr : FLT_MAX;

			// If new path to neighbor is cheaper than the old path
			if (newCostToNeighbor < oldCostToNeighbor) {
				// Save new cost
				HashMapSetIntKey(&costSoFar, Vec2IToHashMapKey(neighbor), &newCostToNeighbor);
				// Calculate priority of neighbor with heuristic parameter (which is Manhattan distance to `to`)
				tmpPrioListItem.priority = newCostToNeighbor + ManhattanDistance(to, neighbor);
				tmpPrioListItem.position = neighbor;
				// Insert neighbor into frontiers
				bool inserted = false;
				for (ID priorityCheckIterator = ListGetFirst(&frontiers); priorityCheckIterator; priorityCheckIterator = ListGetNext(&frontiers, priorityCheckIterator)) {
					PriorityListItem* item = ListGetData(&frontiers, priorityCheckIterator);
					if (tmpPrioListItem.priority < item->priority) {
						ListInsertBefore(&frontiers, priorityCheckIterator, &tmpPrioListItem, NULL);
						inserted = true;
						break;
					}
				}
				if (!inserted) {
					ListAppend(&frontiers, &tmpPrioListItem, NULL);
				}
				// Set the previous position of neighbor as the current position
				tmpCameFrom = frontierItem->position;
				HashMapSetIntKey(&cameFrom, Vec2IToHashMapKey(neighbor), &tmpCameFrom);
			}
		}

		// Remove current position as we're done processing it
		ListRemove(&frontiers, frontierIterator);
	}

	int result;
	// Check if there is a path
	Vec2I* currentCameFrom = HashMapGetIntKey(&cameFrom, Vec2IToHashMapKey(to));
	if (currentCameFrom == NULL) {
		result = XERR_PATH_NOT_FOUND;
	} else {
		result = 0;

		ListClear(outReverseListOfVec2Is);
		// Add `to` to list
		ListAppend(outReverseListOfVec2Is, &to, NULL);
		// Built outReverseListOfVec2Is
		while (currentCameFrom && Vec2IEquals(from, *currentCameFrom) == false) {
			ListAppend(outReverseListOfVec2Is, currentCameFrom, NULL);
			currentCameFrom = HashMapGetIntKey(&cameFrom, Vec2IToHashMapKey(*currentCameFrom));
		}
		// Add `from` to list as well
		if (currentCameFrom) {
			ListAppend(outReverseListOfVec2Is, currentCameFrom, NULL);
		}
	}

	// Cleanup
	HashMapDeinit(&costSoFar);
	HashMapDeinit(&cameFrom);
	ListDeinit(&frontiers);
	return result;
}

void PathfinderMapGridStepsToAnyAngle(List* listOfVec2Is, List* outListOfVec2Is) {
	ListClear(outListOfVec2Is);

	const ID point1Iterator = ListGetFirst(listOfVec2Is);
	Vec2I* point1 = ListGetData(listOfVec2Is, point1Iterator);
	// Add first point
	if (point1) {
		ListAppend(outListOfVec2Is, point1, NULL);
	}
	
	if (point1 == NULL || listOfVec2Is->bucket.size < 2) {
		return;
	}

	Vec2I* prevPoint2 = NULL;
	for (ID point2Iterator = ListGetNext(listOfVec2Is, point1Iterator); point2Iterator; point2Iterator = ListGetNext(listOfVec2Is, point2Iterator)) {
		Vec2I* point2 = ListGetData(listOfVec2Is, point2Iterator);

		const bool eyeSight = Box2DUtils_CheckEyeSight(Vec2FFromVec2I(*point1), Vec2FFromVec2I(*point2), CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF);
		if (ListGetLast(listOfVec2Is) == point2Iterator && eyeSight) {
			// If we are processing the last point and there is an eye sight, add the last point
			ListAppend(outListOfVec2Is, point2, NULL);
		} else if (ListGetLast(listOfVec2Is) == point2Iterator && !eyeSight) {
			// If we are processing the last point and there is no eye sight, add previous and the last point
			if (prevPoint2) {
				ListAppend(outListOfVec2Is, prevPoint2, NULL);
			}
			ListAppend(outListOfVec2Is, point2, NULL);
		} else if (eyeSight) {
			// There is an eye sight, continue iterating
		} else {
			// There is no eye sight, add previous point
			if (prevPoint2) {
				ListAppend(outListOfVec2Is, prevPoint2, NULL);
			}
			point1 = prevPoint2;
		}
		
		prevPoint2 = point2;
	}
}
