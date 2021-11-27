#include "../Object.h"
#include "../Main.h"
#include "../Box2D.h"
#include "../Log.h"
#include "../Pathfinder.h"
#include "../Box2DUtils.h"
#include <stdio.h>
#include <assert.h>

void ObjectEnemy_prePhysics(ComponentEventListener* el) {
	Object* obj = FindObjectOfComponent(el);
	Object* player = FindObjectById(GAME->playerId);

	float distanceToPlayer = 0.0f;

	obj->properties->ai->recalculationStopwatch += GAME->deltaTicks;
	obj->properties->ai->attackStopwatch += GAME->deltaTicks;
	
	if (obj->properties->ai->recalculationPeriod < obj->properties->ai->recalculationStopwatch) {
		switch (obj->properties->ai->mode) {
			case AI_IDLE:
				// If player is close
				if (Vec2F_Distance(obj->position, player->position) < obj->properties->ai->triggerDistance) {
					PathfinderMap_FindPath(&GAME->pathfinderMap, obj->position, player->position, &obj->properties->ai->reversedWaypointList);
					if (1 < List_Length(&obj->properties->ai->reversedWaypointList)) {
						obj->properties->ai->mode = AI_GOING_AFTER_PLAYER;
					}
				}
				break;
			case AI_GOING_AFTER_PLAYER:
				distanceToPlayer = Vec2F_Distance(obj->position, player->position);
				if (distanceToPlayer < 1.2f) {
					if (obj->properties->ai->attackPeriod < obj->properties->ai->attackStopwatch) {
						// If enough time passed for the next attack
						Object* sword = Pool_Mark(&GAME->objects, NULL, NULL);
						ObjectSword_Init(sword, obj->position, FindOffenseMeleeOfObject(obj), true, Vec2F_Sub(player->position, obj->position), 150);
						LOG_INF("Attacking player with melee");
						
						obj->properties->ai->attackStopwatch = 0;
					}
					obj->properties->ai->mode = AI_GOING_AFTER_PLAYER;
				} else if (distanceToPlayer < 2 * obj->properties->ai->triggerDistance) {
					PathfinderMap_FindPath(&GAME->pathfinderMap, obj->position, player->position, &obj->properties->ai->reversedWaypointList);
					if (1 < List_Length(&obj->properties->ai->reversedWaypointList)) {
						obj->properties->ai->mode = AI_GOING_AFTER_PLAYER;
					} else {
						obj->properties->ai->mode = AI_GOING_BACK_TO_HOME;
					}
				} else {
					obj->properties->ai->mode = AI_GOING_BACK_TO_HOME;
				}
				break;
			case AI_GOING_BACK_TO_HOME:
				if (Vec2F_Distance(obj->position, player->position) < obj->properties->ai->triggerDistance) {
					PathfinderMap_FindPath(&GAME->pathfinderMap, obj->position, player->position, &obj->properties->ai->reversedWaypointList);
					if (1 < List_Length(&obj->properties->ai->reversedWaypointList)) {
						obj->properties->ai->mode = AI_GOING_AFTER_PLAYER;
					} else {
						obj->properties->ai->mode = AI_GOING_BACK_TO_HOME;
					}
				} else {
					PathfinderMap_FindPath(&GAME->pathfinderMap, obj->position, obj->properties->ai->homePosition, &obj->properties->ai->reversedWaypointList);
					if (1 < List_Length(&obj->properties->ai->reversedWaypointList)) {
						obj->properties->ai->mode = AI_GOING_BACK_TO_HOME;
					} else {
						obj->properties->ai->mode = AI_IDLE;
					}
				}
				break;
		}

		obj->properties->ai->recalculationStopwatch = 0;
	}

	if (1 < List_Length(&obj->properties->ai->reversedWaypointList)) {
		ID myPositionIterator = List_GetLast(&obj->properties->ai->reversedWaypointList);
		Vec2I* myPosition = List_GetData(&obj->properties->ai->reversedWaypointList, myPositionIterator);
		
		ID targetIterator = List_GetPrev(&obj->properties->ai->reversedWaypointList, myPositionIterator);
		Vec2I* targetPosition = List_GetData(&obj->properties->ai->reversedWaypointList, targetIterator);
		
		if (myPosition && targetPosition && !Vec2I_Equals(*myPosition, *targetPosition)) {
			ComponentPhysics* phy = FindPhysicsOfObject(obj);
			Vec2F direction = Vec2F_Sub(Vec2F_FromVec2I(*targetPosition), obj->position);
			Box2DBodyApplyForceToCenter(phy->body, Vec2F_Mul(Vec2F_Normalize(direction), GAME->deltaTicks * 20.0f), true);
		}
	}
}

// Move this to main loop
void ObjectEnemy_postPhysics(ComponentEventListener *el) {
	Object* obj = FindObjectOfComponent(el);
	if (obj) {
		ComponentDefense* defense = FindDefenseOfObject(obj);
		if (defense && defense->hp <= 0) {
			Array_Append(&GAME->deleteList, &el->super.objId);
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
	(void)descriptor;
	REFLECT_ERROR(Object_Init(obj, position, true));

	ComponentGraphics* gfx = Object_AddGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect){ 2 * GAME->tileWidth, 0, GAME->tileWidth, GAME->tileWidth };
	gfx->txCenter = (Vec2F){ 0.0f, 4.5f };
	gfx->draw = ObjectEnemy_Draw;

	if (GAME->levelType != LEVEL_TYPE_LEVEL_EDITOR) {
		obj->properties->ai = malloc(sizeof(AI));
		assert(obj->properties->ai);
		AI_Init(obj->properties->ai);
		obj->properties->ai->recalculationPeriod = 500;
		obj->properties->ai->recalculationStopwatch = rand() % obj->properties->ai->recalculationPeriod;
		obj->properties->ai->attackPeriod = 500;
		obj->properties->ai->attackStopwatch = rand() % obj->properties->ai->attackPeriod;
		obj->properties->ai->homePosition = position;
		obj->properties->ai->triggerDistance = 6.0f;

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

		ComponentDefense* defense = Object_AddDefense(obj, NULL);
		defense->hp = 100;
		defense->maxHp = 100;

		ComponentOffense* offense = Object_AddOffenseMelee(obj, NULL);
		offense->originator = Pool_GetId(&GAME->objects, obj);
		offense->hp = 10;
		offense->ttl = 100;
	}
	
	return 0;
}
