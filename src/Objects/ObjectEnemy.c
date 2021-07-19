#include "../Object.h"
#include "../Main.h"
#include "../Box2D.h"
#include "../Pathfinder.h"
#include "../Box2DUtils.h"
#include <stdio.h>
#include <assert.h>

void ObjectEnemy_prePhysics(ComponentEventListener* el) {
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
				if (Vec2F_Distance(me->position, player->position) < me->ai->triggerDistance) {
					PathfinderMap_FindPath(&CurrentLevel()->pathfinderMap, me->position, player->position, &me->ai->reversedWaypointList);
					if (1 < List_Length(&me->ai->reversedWaypointList)) {
						me->ai->mode = AI_GOING_AFTER_PLAYER;
					}
				}
				break;
			case AI_GOING_AFTER_PLAYER:
				if (Vec2F_Distance(me->position, player->position) < me->ai->triggerDistance) {
					PathfinderMap_FindPath(&CurrentLevel()->pathfinderMap, me->position, player->position, &me->ai->reversedWaypointList);
					if (1 < List_Length(&me->ai->reversedWaypointList)) {
						me->ai->mode = AI_GOING_AFTER_PLAYER;
					} else {
						me->ai->mode = AI_GOING_BACK_TO_HOME;
					}
				} else {
					me->ai->mode = AI_GOING_BACK_TO_HOME;
				}
				break;
			case AI_GOING_BACK_TO_HOME:
				if (Vec2F_Distance(me->position, player->position) < me->ai->triggerDistance) {
					PathfinderMap_FindPath(&CurrentLevel()->pathfinderMap, me->position, player->position, &me->ai->reversedWaypointList);
					if (1 < List_Length(&me->ai->reversedWaypointList)) {
						me->ai->mode = AI_GOING_AFTER_PLAYER;
					} else {
						me->ai->mode = AI_GOING_BACK_TO_HOME;
					}
				} else {
					PathfinderMap_FindPath(&CurrentLevel()->pathfinderMap, me->position, me->ai->homePosition, &me->ai->reversedWaypointList);
					if (1 < List_Length(&me->ai->reversedWaypointList)) {
						me->ai->mode = AI_GOING_BACK_TO_HOME;
					} else {
						me->ai->mode = AI_IDLE;
					}
				}
				break;
		}
	}

	if (1 < List_Length(&me->ai->reversedWaypointList)) {
		ID myPositionIterator = List_GetLast(&me->ai->reversedWaypointList);
		Vec2I* myPosition = List_GetData(&me->ai->reversedWaypointList, myPositionIterator);
		
		ID targetIterator = List_GetPrev(&me->ai->reversedWaypointList, myPositionIterator);
		Vec2I* targetPosition = List_GetData(&me->ai->reversedWaypointList, targetIterator);
		
		if (myPosition && targetPosition && !Vec2I_Equals(*myPosition, *targetPosition)) {
			ComponentPhysics* phy = FindPhysicsOfObject(me);
			Vec2F direction = Vec2F_Sub(Vec2F_FromVec2I(*targetPosition), me->position);
			Box2DBodyApplyForceToCenter(phy->body, Vec2F_Mul(Vec2F_Normalize(direction), DeltaTicks() * 20.0f), true);
		}
	}
}

// Move this to main loop
void ObjectEnemy_postPhysics(ComponentEventListener *el) {
	Object* obj = FindObjectOfComponent(el);
	if (obj) {
		ComponentDefense* defense = FindDefenseOfObject(obj);
		if (defense && defense->hp <= 0) {
			Array_Append(&CurrentLevel()->deleteList, &el->super.objId);
		}
	}
}

int ObjectEnemy_Init(Object* obj, Vec2F position, const char* descriptor) {
	PROPAGATE_ERROR(Object_Init(obj, position));

	ComponentEventListener* el = Object_AddEventListener(obj, NULL);
	el->prePhysics = ObjectEnemy_prePhysics;
	el->postGraphics = ObjectEnemy_postPhysics;

	ID phyId = 0;
	ComponentPhysics* phy = Object_AddPhysics(obj, &phyId);
	phy->body = Box2DUtils_CreateDynamicDisk(
		phyId,
		position,
		true, // allowSleep
		CATEGORY_ENEMY,
		0.2083f, // Radius
		10.0f, // Mass
		10.0f // Damping
	);

	ComponentGraphics* gfx = Object_AddGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect){2 * TILE_WIDTH, 0, TILE_WIDTH, TILE_WIDTH };
	gfx->txCenter = (Vec2F){ 0.0f, 4.5f };

	ComponentDefense* defense = Object_AddDefense(obj, NULL);
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
