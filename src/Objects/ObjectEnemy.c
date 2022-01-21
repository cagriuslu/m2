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

void ObjectEnemy_onHit(ComponentDefense* def) {
	Object* obj = Pool_GetById(&GAME->objects, def->super.objId); XASSERT(obj);
	obj->ex->value.enemy.onHitColorModTtl = 0.10f;
}

void ObjectEnemy_onDeath(ComponentDefense* def) {
	Game_DeleteList_Add(def->super.objId);
}

static void ObjectEnemy_postPhysics(ComponentMonitor* monitor) {
	Object* obj = Pool_GetById(&GAME->objects, monitor->super.objId); XASSERT(obj);
	ComponentPhysique* phy = Pool_GetById(&GAME->physics, obj->physique); XASSERT(phy);
	// We must call time before other signals
	StateMachine_ProcessTime(&obj->ex->value.enemy.stateMachineCharacterAnimation, GAME->deltaTicks / 1000.0f);
	Vec2F velocity = Box2DBodyGetLinearVelocity(phy->body);
	if (fabsf(velocity.x) < 0.5000f && fabsf(velocity.y) < 0.5000f) {
		StateMachine_ProcessSignal(&obj->ex->value.enemy.stateMachineCharacterAnimation, SIG_CHARANIM_STOP);
	} else if (fabsf(velocity.x) < fabsf(velocity.y)) {
		if (0 < velocity.y) {
			StateMachine_ProcessSignal(&obj->ex->value.enemy.stateMachineCharacterAnimation, SIG_CHARANIM_WALKDOWN);
		} else {
			StateMachine_ProcessSignal(&obj->ex->value.enemy.stateMachineCharacterAnimation, SIG_CHARANIM_WALKUP);
		}
	} else {
		if (0 < velocity.x) {
			StateMachine_ProcessSignal(&obj->ex->value.enemy.stateMachineCharacterAnimation, SIG_CHARANIM_WALKRIGHT);
		} else {
			StateMachine_ProcessSignal(&obj->ex->value.enemy.stateMachineCharacterAnimation, SIG_CHARANIM_WALKLEFT);
		}
	}
}

void ObjectEnemy_Draw(ComponentGraphic* gfx, SDL_Color* mod) {
	Object* obj = Game_FindObjectById(gfx->super.objId); XASSERT(obj);
	if (0.0f < obj->ex->value.enemy.onHitColorModTtl) {
		SDL_Color hitMod = {255, 255, 255};
		ComponentGraphic_DefaultDraw(gfx, &hitMod);
		obj->ex->value.enemy.onHitColorModTtl -= GAME->deltaTicks / 1000.0f;
	} else {
		ComponentGraphic_DefaultDraw(gfx, NULL);
	}
	ComponentDefense* defense = Object_GetDefense(obj); XASSERT(defense);
	ComponentGraphic_DefaultDrawHealthBar(gfx, (float) defense->hp / defense->maxHp);
}

int ObjectEnemy_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position) {
	XERR_REFLECT(Object_Init(obj, position, true));
	obj->ex->type = CFG_OBJTYP_ENEMY;
	obj->ex->value.enemy.chr = cfg;
	// TODO implement CharacterState

	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = cfg->mainTexture->textureRect;
	gfx->center_px = cfg->mainTexture->objCenter_px;
	gfx->draw = ObjectEnemy_Draw;
	gfx->motionBlurEnabled = true;
	gfx->prevObjGfxOriginWRTScreenCenter_px = ComponentGraphic_GraphicsOriginWRTScreenCenter_px(position, cfg->mainTexture->objCenter_px);

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
	el->postPhysics = ObjectEnemy_postPhysics;

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
	defense->onHit = ObjectEnemy_onHit;
	defense->onDeath = ObjectEnemy_onDeath;

	StateMachineCharacterAnimation_Init(&obj->ex->value.enemy.stateMachineCharacterAnimation, cfg, gfx);

	return 0;
}
