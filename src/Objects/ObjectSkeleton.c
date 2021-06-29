#include "../Object.h"
#include "../Main.h"
#include "../Box2D.h"
#include "../Pathfinder.h"
#include "../Box2DUtils.h"
#include <stdio.h>
#include <assert.h>

void ObjectSkeleton_prePhysics(EventListenerComponent* el) {
	Object* me = FindObjectOfComponent(el);
	me->ai->waypointRecalculationStopwatch += DeltaTicks();
	if (1000 < me->ai->waypointRecalculationStopwatch) {
		Object* player = FindObjectById(CurrentLevel()->playerId);

		List gridSteps;
		ListInit(&gridSteps, sizeof(Vec2I));
		int pathfinderResult = PathfinderMapFindGridSteps(&CurrentLevel()->pathfinderMap, me->position, player->position, &gridSteps);
		if (pathfinderResult == 0) {
			PathfinderMapGridStepsToAnyAngle(&gridSteps, &me->ai->reversedWaypointList);
		} else {
			ListClear(&me->ai->reversedWaypointList);
		}
		ListDeinit(&gridSteps);
		
		me->ai->waypointRecalculationStopwatch -= 1000;
	}

	if (1 < me->ai->reversedWaypointList.bucket.size) {
		ID myPositionIterator = ListGetLast(&me->ai->reversedWaypointList);
		ID targetIterator = ListGetPrev(&me->ai->reversedWaypointList, myPositionIterator);
		Vec2I* targetPosition = ListGetData(&me->ai->reversedWaypointList, targetIterator);
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
			Array_Append(&CurrentLevel()->deleteList, &el->super.objId);
		}
	}
}

int ObjectSkeletonInit(Object* obj, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj, position));

	EventListenerComponent* el = ObjectAddEventListener(obj, NULL);
	el->prePhysics = ObjectSkeleton_prePhysics;
	el->postGraphics = ObjectSkeleton_postPhysics;

	ID phyId = 0;
	PhysicsComponent* phy = ObjectAddPhysics(obj, &phyId);
	phy->body = Box2DUtils_CreateDynamicDisk(
		phyId,
		position,
		true, // allowSleep
		CATEGORY_ENEMY,
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
	AI_Init(obj->ai);
	obj->ai->waypointRecalculationStopwatch = 1000 + rand() % 1000;
	
	return 0;
}
