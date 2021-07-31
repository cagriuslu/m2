#include "../Object.h"
#include "../Main.h"
#include "../Box2D.h"
#include "../Pathfinder.h"
#include "../Box2DUtils.h"
#include <stdio.h>
#include <assert.h>

void ObjectEnemy_prePhysics(ComponentEventListener* el) {
	Object* obj = FindObjectOfComponent(el);
	Object* player = FindObjectById(CurrentLevel()->playerId);

	Stopwatch* recalculationStopwatchPtr = Object_GetPrePhysicsStopwatchPtr(obj, 0);
	if (obj->ai->recalculationPeriod < *recalculationStopwatchPtr) {
		switch (obj->ai->mode) {
			case AI_IDLE:
				// If player is close
				if (Vec2F_Distance(obj->position, player->position) < obj->ai->triggerDistance) {
					PathfinderMap_FindPath(&CurrentLevel()->pathfinderMap, obj->position, player->position, &obj->ai->reversedWaypointList);
					if (1 < List_Length(&obj->ai->reversedWaypointList)) {
						obj->ai->mode = AI_GOING_AFTER_PLAYER;
					}
				}
				break;
			case AI_GOING_AFTER_PLAYER:
				if (Vec2F_Distance(obj->position, player->position) < 2 * obj->ai->triggerDistance) {
					PathfinderMap_FindPath(&CurrentLevel()->pathfinderMap, obj->position, player->position, &obj->ai->reversedWaypointList);
					if (1 < List_Length(&obj->ai->reversedWaypointList)) {
						obj->ai->mode = AI_GOING_AFTER_PLAYER;
					} else {
						obj->ai->mode = AI_GOING_BACK_TO_HOME;
					}
				} else {
					obj->ai->mode = AI_GOING_BACK_TO_HOME;
				}
				break;
			case AI_GOING_BACK_TO_HOME:
				if (Vec2F_Distance(obj->position, player->position) < obj->ai->triggerDistance) {
					PathfinderMap_FindPath(&CurrentLevel()->pathfinderMap, obj->position, player->position, &obj->ai->reversedWaypointList);
					if (1 < List_Length(&obj->ai->reversedWaypointList)) {
						obj->ai->mode = AI_GOING_AFTER_PLAYER;
					} else {
						obj->ai->mode = AI_GOING_BACK_TO_HOME;
					}
				} else {
					PathfinderMap_FindPath(&CurrentLevel()->pathfinderMap, obj->position, obj->ai->homePosition, &obj->ai->reversedWaypointList);
					if (1 < List_Length(&obj->ai->reversedWaypointList)) {
						obj->ai->mode = AI_GOING_BACK_TO_HOME;
					} else {
						obj->ai->mode = AI_IDLE;
					}
				}
				break;
		}

		*recalculationStopwatchPtr = 0;
	}

	if (1 < List_Length(&obj->ai->reversedWaypointList)) {
		ID myPositionIterator = List_GetLast(&obj->ai->reversedWaypointList);
		Vec2I* myPosition = List_GetData(&obj->ai->reversedWaypointList, myPositionIterator);
		
		ID targetIterator = List_GetPrev(&obj->ai->reversedWaypointList, myPositionIterator);
		Vec2I* targetPosition = List_GetData(&obj->ai->reversedWaypointList, targetIterator);
		
		if (myPosition && targetPosition && !Vec2I_Equals(*myPosition, *targetPosition)) {
			ComponentPhysics* phy = FindPhysicsOfObject(obj);
			Vec2F direction = Vec2F_Sub(Vec2F_FromVec2I(*targetPosition), obj->position);
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

void ObjectEnemy_Draw(ComponentGraphics* gfx) {
	GraphicsComponent_DefaultDraw(gfx);

	Object* obj = FindObjectOfComponent(gfx);
	ComponentDefense* defense = FindDefenseOfObject(obj);	
	if (obj && defense) {
		GraphicsComponent_DefaultDrawHealthBar(gfx, (float)defense->hp / defense->maxHp);
	}
}

int ObjectEnemy_Init(Object* obj, Vec2F position, const char* descriptor) {
	PROPAGATE_ERROR(Object_Init(obj, position, false));

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
	gfx->draw = ObjectEnemy_Draw;

	ComponentDefense* defense = Object_AddDefense(obj, NULL);
	defense->hp = 100;
	defense->maxHp = 100;

	obj->ai = malloc(sizeof(AI));
	assert(obj->ai);
	AI_Init(obj->ai);
	obj->ai->recalculationPeriod = 500;
	obj->ai->homePosition = position;
	obj->ai->triggerDistance = 6.0f;

	Object_AddPrePhysicsStopwatches(obj, 1);
	Stopwatch* recalculationStopwatchPtr = Object_GetPrePhysicsStopwatchPtr(obj, 0);
	*recalculationStopwatchPtr = rand() % obj->ai->recalculationPeriod;
	
	return 0;
}
