#include "../Object.h"
#include "../Game.h"
#include "../Box2D.h"
#include "../Def.h"
#include "../Pathfinder.h"
#include "../Box2DUtils.h"

void ObjectEnemy_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId); XASSERT(obj);
	Object* player = Game_FindObjectById(GAME->playerId); XASSERT(player);

	float distanceToPlayer = 0.0f;

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
			ComponentPhysique* phy = Object_GetPhysique(obj);
			Vec2F direction = Vec2F_Sub(Vec2F_FromVec2I(*targetPosition), obj->position);
			Box2DBodyApplyForceToCenter(phy->body, Vec2F_Mul(Vec2F_Normalize(direction), GAME->deltaTicks * 20.0f), true);
		}
	}
}

void ObjectEnemy_onDeath(ComponentDefense* def) {
	Game_DeleteList_Add(def->super.objId);
}

void ObjectEnemy_Draw(ComponentGraphic* gfx) {
	ComponentGraphic_DefaultDraw(gfx);

	Object* obj = Game_FindObjectById(gfx->super.objId); XASSERT(obj);
	ComponentDefense* defense = Object_GetDefense(obj); XASSERT(defense);
	ComponentGraphic_DefaultDrawHealthBar(gfx, (float) defense->hp / defense->maxHp);
}

int ObjectEnemy_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position) {
	XERR_REFLECT(Object_Init(obj, position, true));

	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = cfg->texture->textureRect;
	gfx->center_px = cfg->texture->objCenter_px;
	gfx->draw = ObjectEnemy_Draw;
	gfx->motionBlurEnabled = true;
	gfx->prevDrawPosition = position;

	obj->ex->type = CFG_OBJTYP_ENEMY;
	AI* ai = malloc(sizeof(AI)); // TODO error check, who uses malloc anyways
	AI_Init(ai);
	ai->recalculationPeriod = 500;
	ai->recalculationStopwatch = rand() % ai->recalculationPeriod;
	ai->attackPeriod = 500;
	ai->attackStopwatch = rand() % ai->attackPeriod;
	ai->homePosition = position;
	ai->triggerDistance = 6.0f;
	obj->ex->value.enemy.ai = ai;

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->prePhysics = ObjectEnemy_prePhysics;

	ComponentPhysique* phy = Object_AddPhysique(obj);
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
	defense->onDeath = ObjectEnemy_onDeath;

	return 0;
}
