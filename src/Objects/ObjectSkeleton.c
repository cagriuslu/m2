#include "../Object.h"
#include "../Main.h"
#include "../Box2DWrapper.h"
#include "../Pathfinder.h"
#include "../Box2DUtils.h"
#include <stdio.h>
#include <assert.h>

void ObjectSkeleton_prePhysics(EventListenerComponent* el) {
	Object* me = FindObjectOfComponent(el);
	me->ai->waypointRecalculationStopwatch += DeltaTicks();
	if (500 < me->ai->waypointRecalculationStopwatch) {
		Object* player = FindObjectById(CurrentLevel()->playerId);

		List gridSteps;
		ListInit(&gridSteps, sizeof(Vec2I));
		int pathfinderResult = PathfinderMapFindGridSteps(&CurrentLevel()->pathfinderMap, me->position, player->position, &gridSteps);
		if (pathfinderResult == 0) {
			PathfinderMapGridStepsToAnyAngle(&gridSteps, 0.5f, &me->ai->reversedVec2IWaypointList);
		} else {
			ListClear(&me->ai->reversedVec2IWaypointList);
		}
		ListDeinit(&gridSteps);
		
		me->ai->waypointRecalculationStopwatch -= 500;
	}

	if (1 < me->ai->reversedVec2IWaypointList.bucket.size) {
		uint64_t myPositionIterator = ListGetLast(&me->ai->reversedVec2IWaypointList);
		uint64_t targetIterator = ListGetPrev(&me->ai->reversedVec2IWaypointList, myPositionIterator);
		Vec2I* targetPosition = ListGetData(&me->ai->reversedVec2IWaypointList, targetIterator);
		if (targetPosition) {
			PhysicsComponent* phy = FindPhysicsOfObject(me);
			Vec2F direction = Vec2FSub(Vec2FFromVec2I(*targetPosition), me->position);
			Box2DBodyApplyForceToCenter(phy->body, Vec2FMul(Vec2FNormalize(direction), DeltaTicks() * 20.0f), true);
		}
	}
}

// Move this to main loop
void ObjectSkeleton_postPhysics(EventListenerComponent *el) {
	Object* obj = FindObjectOfComponent(el);
	if (obj) {
		ComponentDefense* defense = FindDefenseOfObject(obj);
		if (defense && defense->hp <= 0) {
			ArrayAppend(&CurrentLevel()->deleteList, &el->super.objId);
		}
	}
}

int ObjectSkeletonInit(Object* obj, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj, position));

	EventListenerComponent* el = ObjectAddEventListener(obj, NULL);
	el->prePhysics = ObjectSkeleton_prePhysics;
	el->postGraphics = ObjectSkeleton_postPhysics;

	uint64_t phyId = 0;
	PhysicsComponent* phy = ObjectAddPhysics(obj, &phyId);
	phy->body = Box2DUtilsCreateDynamicDisk(
		phyId,
		position,
		ALLOW_SLEEP,
		ENEMY_CATEGORY,
		0.2083f, // Radius
		10.0f, // Mass
		10.0f // Damping
	);

	GraphicsComponent* gfx = ObjectAddGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect){2 * TILE_WIDTH, 0, TILE_WIDTH, TILE_WIDTH };
	gfx->txCenter = (Vec2F){ 0.0f, 4.5f };

	ComponentDefense* defense = ObjectAddDefense(obj, NULL);
	defense->hp = 100;
	defense->maxHp = 100;

	obj->ai = malloc(sizeof(AI));
	assert(obj->ai);
	AIInit(obj->ai);
	
	return 0;
}
