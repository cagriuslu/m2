#include "../Object.h"
#include "../Game.h"
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

	if (obj->ex && obj->ex->type == CFG_OBJTYP_ENEMY) {
		AI* ai = obj->ex->value.enemy.ai;
		
		ai->recalculationStopwatch += GAME->deltaTicks;
		ai->attackStopwatch += GAME->deltaTicks;

		if (ai->recalculationPeriod < ai->recalculationStopwatch) {
			switch (ai->mode) {
				case AI_IDLE:
					// If player is close
					if (Vec2F_Distance(obj->position, player->position) < ai->triggerDistance) {
						PathfinderMap_FindPath(&GAME->pathfinderMap, obj->position, player->position, &ai->reversedWaypointList);
						if (1 < List_Length(&ai->reversedWaypointList)) {
							ai->mode = AI_GOING_AFTER_PLAYER;
						}
					}
					break;
				case AI_GOING_AFTER_PLAYER:
					distanceToPlayer = Vec2F_Distance(obj->position, player->position);
					if (distanceToPlayer < 1.2f) {
						if (ai->attackPeriod < ai->attackStopwatch) {
							// If enough time passed for the next attack
							Object* sword = Pool_Mark(&GAME->objects, NULL, NULL);
							ObjectMelee_InitFromCfg(sword, &CFG_MELEEWPN_SWORD.melee, el->super.objId, obj->position, Vec2F_Sub(player->position, obj->position));
							LOG_INF("Attacking player with melee");

							ai->attackStopwatch = 0;
						}
						ai->mode = AI_GOING_AFTER_PLAYER;
					} else if (distanceToPlayer < 2 * ai->triggerDistance) {
						PathfinderMap_FindPath(&GAME->pathfinderMap, obj->position, player->position, &ai->reversedWaypointList);
						if (1 < List_Length(&ai->reversedWaypointList)) {
							ai->mode = AI_GOING_AFTER_PLAYER;
						} else {
							ai->mode = AI_GOING_BACK_TO_HOME;
						}
					} else {
						ai->mode = AI_GOING_BACK_TO_HOME;
					}
					break;
				case AI_GOING_BACK_TO_HOME:
					if (Vec2F_Distance(obj->position, player->position) < ai->triggerDistance) {
						PathfinderMap_FindPath(&GAME->pathfinderMap, obj->position, player->position, &ai->reversedWaypointList);
						if (1 < List_Length(&ai->reversedWaypointList)) {
							ai->mode = AI_GOING_AFTER_PLAYER;
						} else {
							ai->mode = AI_GOING_BACK_TO_HOME;
						}
					} else {
						PathfinderMap_FindPath(&GAME->pathfinderMap, obj->position, ai->homePosition, &ai->reversedWaypointList);
						if (1 < List_Length(&ai->reversedWaypointList)) {
							ai->mode = AI_GOING_BACK_TO_HOME;
						} else {
							ai->mode = AI_IDLE;
						}
					}
					break;
			}

			ai->recalculationStopwatch = 0;
		}

		if (1 < List_Length(&ai->reversedWaypointList)) {
			ID myPositionIterator = List_GetLast(&ai->reversedWaypointList);
			Vec2I* myPosition = List_GetData(&ai->reversedWaypointList, myPositionIterator);

			ID targetIterator = List_GetPrev(&ai->reversedWaypointList, myPositionIterator);
			Vec2I* targetPosition = List_GetData(&ai->reversedWaypointList, targetIterator);

			if (myPosition && targetPosition && !Vec2I_Equals(*myPosition, *targetPosition)) {
				ComponentPhysics* phy = FindPhysicsOfObject(obj);
				Vec2F direction = Vec2F_Sub(Vec2F_FromVec2I(*targetPosition), obj->position);
				Box2DBodyApplyForceToCenter(phy->body, Vec2F_Mul(Vec2F_Normalize(direction), GAME->deltaTicks * 20.0f), true);
			}
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

int ObjectEnemy_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position) {
	REFLECT_ERROR(Object_Init(obj, position, true));

	ComponentGraphics* gfx = Object_AddGraphics(obj);
	gfx->textureRect = cfg->texture->textureRect;
	gfx->center_px = cfg->texture->objCenter_px;
	gfx->draw = ObjectEnemy_Draw;

	obj->ex->type = CFG_OBJTYP_ENEMY;
	AI* ai = malloc(sizeof(AI));
	assert(ai);
	AI_Init(ai);
	ai->recalculationPeriod = 500;
	ai->recalculationStopwatch = rand() % ai->recalculationPeriod;
	ai->attackPeriod = 500;
	ai->attackStopwatch = rand() % ai->attackPeriod;
	ai->homePosition = position;
	ai->triggerDistance = 6.0f;
	obj->ex->value.enemy.ai = ai;

	ComponentEventListener* el = Object_AddEventListener(obj);
	el->prePhysics = ObjectEnemy_prePhysics;
	el->postPhysics = ObjectEnemy_postPhysics;

	ComponentPhysics* phy = Object_AddPhysics(obj);
	phy->body = Box2DUtils_CreateDynamicDisk(
		Pool_GetId(&GAME->physics, phy),
		position,
		true, // allowSleep
		CATEGORY_ENEMY,
		0.2083f, // Radius
		10.0f, // Mass
		10.0f // Damping
	);

	ComponentDefense* defense = Object_AddDefense(obj);
	defense->hp = 100;
	defense->maxHp = 100;

	ComponentOffense* offense = Object_AddOffense(obj);
	offense->originator = Pool_GetId(&GAME->objects, obj);
	offense->hp = 10;
	offense->ttl = 100;

	return 0;
}
