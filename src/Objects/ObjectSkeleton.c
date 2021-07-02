#include "../Object.h"
#include "../Main.h"
#include "../Box2D.h"
#include "../Pathfinder.h"
#include "../Box2DUtils.h"
#include <stdio.h>
#include <assert.h>

void ObjectSkeleton_prePhysics(EventListenerComponent* el) {
	Object* me = FindObjectOfComponent(el);
	Object* player = FindObjectById(CurrentLevel()->playerId);
	
	bool recalculate = false;
	me->ai->waypointRecalculationStopwatch += DeltaTicks();
	if (me->ai->recalculationPeriod < me->ai->waypointRecalculationStopwatch) {
		recalculate = true;
		me->ai->waypointRecalculationStopwatch -= me->ai->recalculationPeriod;
	}

	if (recalculate) {
		switch (me->ai->mode) {
			case AI_IDLE:
				// If player is close
				if (Vec2FDistance(me->position, player->position) < me->ai->triggerDistance) {
					PathfinderMapFindPath(&CurrentLevel()->pathfinderMap, me->position, player->position, &me->ai->reversedWaypointList);
					if (1 < ListLength(&me->ai->reversedWaypointList)) {
						me->ai->mode = AI_GOING_AFTER_PLAYER;
					}
				}
				break;
			case AI_GOING_AFTER_PLAYER:
				if (Vec2FDistance(me->position, player->position) < me->ai->triggerDistance) {
					PathfinderMapFindPath(&CurrentLevel()->pathfinderMap, me->position, player->position, &me->ai->reversedWaypointList);
					if (1 < ListLength(&me->ai->reversedWaypointList)) {
						me->ai->mode = AI_GOING_AFTER_PLAYER;
					} else {
						me->ai->mode = AI_GOING_BACK_TO_HOME;
					}
				} else {
					me->ai->mode = AI_GOING_BACK_TO_HOME;
				}
				break;
			case AI_GOING_BACK_TO_HOME:
				if (Vec2FDistance(me->position, player->position) < me->ai->triggerDistance) {
					PathfinderMapFindPath(&CurrentLevel()->pathfinderMap, me->position, player->position, &me->ai->reversedWaypointList);
					if (1 < ListLength(&me->ai->reversedWaypointList)) {
						me->ai->mode = AI_GOING_AFTER_PLAYER;
					} else {
						me->ai->mode = AI_GOING_BACK_TO_HOME;
					}
				} else {
					PathfinderMapFindPath(&CurrentLevel()->pathfinderMap, me->position, me->ai->homePosition, &me->ai->reversedWaypointList);
					if (1 < ListLength(&me->ai->reversedWaypointList)) {
						me->ai->mode = AI_GOING_BACK_TO_HOME;
					} else {
						me->ai->mode = AI_IDLE;
					}
				}
				break;
		}
	}

	if (1 < ListLength(&me->ai->reversedWaypointList)) {
		ID myPositionIterator = ListGetLast(&me->ai->reversedWaypointList);
		Vec2I* myPosition = ListGetData(&me->ai->reversedWaypointList, myPositionIterator);
		
		ID targetIterator = ListGetPrev(&me->ai->reversedWaypointList, myPositionIterator);
		Vec2I* targetPosition = ListGetData(&me->ai->reversedWaypointList, targetIterator);
		
		if (myPosition && targetPosition && !Vec2IEquals(*myPosition, *targetPosition)) {
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
	obj->ai->recalculationPeriod = 500;
	obj->ai->homePosition = position;
	obj->ai->triggerDistance = 8.0f;
	obj->ai->waypointRecalculationStopwatch = rand() % obj->ai->recalculationPeriod;
	
	return 0;
}
