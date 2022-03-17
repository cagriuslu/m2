#include <m2/Automaton.hh>
#include <m2/Object.hh>
#include <m2/Game.hh>
#include <m2/Pathfinder.hh>
#include "../ARPG_Cfg.hh"
#include "../ARPG_Object.hh"

#define ALARM_DURATION(recalcPeriod) ((recalcPeriod) / 2.0f + (recalcPeriod) * randf() * 1.5f)

void* AiChaseState_Idle(struct _Automaton* am, int signal);
void* AiChaseState_Triggered(struct _Automaton* am, int signal);
void* AiChaseState_GaveUp(struct _Automaton* am, int signal);

M2Err AutomatonAiChase_Init(Automaton *am, void* obj, void* phy) {
	M2ERR_REFLECT(Automaton_Init(am));
	am->currState = AiChaseState_Idle;
	am->userData_obj = obj;
	am->userData_phy = phy;
	Automaton_ProcessSignal(am, SIG_ENTER);
	return M2OK;
}

void* AiChaseState_Idle(struct _Automaton* am, int signal) {
	Object* obj = static_cast<Object *>(am->userData_obj);
	AiState* aiState = &(AS_ENEMYDATA(obj->data)->aiState);
	Object* player = Game_FindObjectById(GAME->playerId);
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(am, ALARM_DURATION(aiState->cfg->recalculationPeriod_s));
			return NULL;
		case SIG_ALARM:
			// Check if player is close
			if (Vec2F_DistanceSquared(obj->position, player->position) < aiState->cfg->triggerDistanceSquared_m) {
				// Check if path exists
				if (PathfinderMap_FindPath(&GAME->pathfinderMap, obj->position, player->position, &aiState->reversedWaypointList) == M2OK) {
					return (void*)AiChaseState_Triggered;
				}
			}
			Automaton_ArmAlarm(am, ALARM_DURATION(aiState->cfg->recalculationPeriod_s));
			return NULL;
		case SIG_AI_PREPHYSICS:
			// TODO implement small patrol
			return NULL;
		default:
			return NULL;
	}
}

void AiChase_AttackIfCloseEnough(Object* obj, Object* player) {
	CharacterState* charState = &(AS_ENEMYDATA(obj->data)->characterState);
	AiState* aiState = &(AS_ENEMYDATA(obj->data)->aiState);
	M2ASSERT(player);
	// If player is close enough
	if (Vec2F_DistanceSquared(obj->position, player->position) < aiState->cfg->attackDistanceSquared_m) {
		// Based on what the capability is
		switch (aiState->cfg->capability) {
			case CFG_AI_CAPABILITY_RANGED: {
				RangedWeaponState* weaponState = &charState->rangedWeaponState; M2ASSERT(weaponState->cfg);
				// If the weapon cooled down
				if (weaponState->cfg->cooldown_s <= weaponState->cooldownCounter_s) {
					Object* projectile = static_cast<Object *>(Pool_Mark(&GAME->objects, NULL, NULL));
					ObjectProjectile_InitFromCfg(
						projectile,
						&weaponState->cfg->projectile,
						Pool_GetId(&GAME->objects, obj),
						obj->position,
						Vec2F_Sub(player->position, obj->position)
					);
					// TODO Knockback maybe?
					weaponState->cooldownCounter_s = 0.0f;
				}
				break;
			}
			case CFG_AI_CAPABILITY_MELEE: {
				MeleeWeaponState* weaponState = &charState->meleeWeaponState; M2ASSERT(weaponState->cfg);
				// If the weapon cooled down
				if (weaponState->cfg->cooldown_s <= weaponState->cooldownCounter_s) {
					Object* melee = static_cast<Object *>(Pool_Mark(&GAME->objects, NULL, NULL));
					ObjectMelee_InitFromCfg(
						melee,
						&weaponState->cfg->melee,
						Pool_GetId(&GAME->objects, obj),
						obj->position,
						Vec2F_Sub(player->position, obj->position)
					);
					weaponState->cooldownCounter_s = 0.0f;
				}
				break;
			}
			case CFG_AI_CAPABILITY_EXPLOSIVE: {
				ExplosiveWeaponState* weaponState = &charState->explosiveWeaponState; M2ASSERT(weaponState->cfg);
				// If the weapon cooled down
				if (weaponState->cfg->cooldown_s <= weaponState->cooldownCounter_s) {
					Object* explosive = static_cast<Object *>(Pool_Mark(&GAME->objects, NULL, NULL));
					ObjectExplosive_InitFromCfg(
						explosive,
						&weaponState->cfg->explosive,
						Pool_GetId(&GAME->objects, obj),
						obj->position,
						Vec2F_Sub(player->position, obj->position)
					);
					// TODO knockback
					weaponState->cooldownCounter_s = 0.0f;
				}
				break;
			}
			case CFG_AI_CAPABILITY_KAMIKAZE: {
				// TODO
				break;
			}
			default:
				break;
		}
	}
}

void* AiChaseState_Triggered(struct _Automaton* am, int signal) {
	Object* obj = static_cast<Object *>(am->userData_obj);
	AiState* aiState = &(AS_ENEMYDATA(obj->data)->aiState);
	ComponentPhysique* phy = static_cast<ComponentPhysique *>(am->userData_phy);
	Object* player = Game_FindObjectById(GAME->playerId);
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(am, ALARM_DURATION(aiState->cfg->recalculationPeriod_s));
			return NULL;
		case SIG_ALARM: {
			// Check if player is still close
			if (Vec2F_DistanceSquared(obj->position, player->position) < aiState->cfg->giveUpDistanceSquared_m) {
				// Recalculate path to player
				PathfinderMap_FindPath(&GAME->pathfinderMap, obj->position, player->position, &aiState->reversedWaypointList);
			} else {
				// Check if path to homePosition exists
				if (PathfinderMap_FindPath(&GAME->pathfinderMap, obj->position, aiState->homePosition, &aiState->reversedWaypointList) == M2OK) {
					return (void*)AiChaseState_GaveUp;
				}
			}
			Automaton_ArmAlarm(am, ALARM_DURATION(aiState->cfg->recalculationPeriod_s));
			return NULL;
		}
		case SIG_AI_PREPHYSICS:
			if (1 < List_Length(&aiState->reversedWaypointList)) {
				ID objPositionIt = List_GetLast(&aiState->reversedWaypointList);
				Vec2I* objPosition = static_cast<Vec2I *>(List_GetData(&aiState->reversedWaypointList, objPositionIt));
				ID targetPositionIt = List_GetPrev(&aiState->reversedWaypointList, objPositionIt);
				Vec2I* targetPosition = static_cast<Vec2I *>(List_GetData(&aiState->reversedWaypointList,
																		  targetPositionIt));
				if (objPosition && targetPosition && !Vec2I_Equals(*objPosition, *targetPosition)) {
					Vec2F objPositionF = Vec2F_FromVec2I(*objPosition);
					Vec2F targetPositionF = Vec2F_FromVec2I(*targetPosition);
					Vec2F direction = Vec2F_Normalize(Vec2F_Sub(targetPositionF, objPositionF));
					Vec2F force = Vec2F_Mul(direction, GAME->deltaTicks_ms * AS_ENEMYDATA(obj->data)->characterState.cfg->walkSpeed);
					Box2DBodyApplyForceToCenter(phy->body, force, true);
				}
			}
			AiChase_AttackIfCloseEnough(obj, player);
			return NULL;
		default:
			return NULL;
	}
}

void* AiChaseState_GaveUp(struct _Automaton* am, int signal) {
	Object* obj = static_cast<Object *>(am->userData_obj);
	AiState* aiState = &(AS_ENEMYDATA(obj->data)->aiState);
	ComponentPhysique* phy = static_cast<ComponentPhysique *>(am->userData_phy);
	Object* player = Game_FindObjectById(GAME->playerId);
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(am, ALARM_DURATION(aiState->cfg->recalculationPeriod_s));
			return NULL;
		case SIG_EXIT:
			return NULL;
		case SIG_ALARM:
			// Check if player is close
			if (Vec2F_DistanceSquared(obj->position, player->position) < aiState->cfg->triggerDistanceSquared_m) {
				// Check if path to player exists
				if (PathfinderMap_FindPath(&GAME->pathfinderMap, obj->position, player->position, &aiState->reversedWaypointList) == M2OK) {
					return (void*)AiChaseState_Triggered;
				}
			} else {
				// Check if obj arrived to homePosition
				if (Vec2F_DistanceSquared(obj->position, aiState->homePosition) < 1.0f) {
					return (void*)AiChaseState_Idle;
				} else {
					// Recalculate path to homePosition
					PathfinderMap_FindPath(&GAME->pathfinderMap, obj->position, aiState->homePosition, &aiState->reversedWaypointList);
				}
			}
			Automaton_ArmAlarm(am, ALARM_DURATION(aiState->cfg->recalculationPeriod_s));
			return NULL;
		case SIG_AI_PREPHYSICS:
			if (1 < List_Length(&aiState->reversedWaypointList)) {
				ID objPositionIt = List_GetLast(&aiState->reversedWaypointList);
				Vec2I* objPosition = static_cast<Vec2I *>(List_GetData(&aiState->reversedWaypointList, objPositionIt));
				ID targetPositionIt = List_GetPrev(&aiState->reversedWaypointList, objPositionIt);
				Vec2I* targetPosition = static_cast<Vec2I *>(List_GetData(&aiState->reversedWaypointList,
																		  targetPositionIt));
				if (objPosition && targetPosition && !Vec2I_Equals(*objPosition, *targetPosition)) {
					Vec2F objPositionF = Vec2F_FromVec2I(*objPosition);
					Vec2F targetPositionF = Vec2F_FromVec2I(*targetPosition);
					Vec2F direction = Vec2F_Normalize(Vec2F_Sub(targetPositionF, objPositionF));
					Vec2F force = Vec2F_Mul(direction, GAME->deltaTicks_ms * AS_ENEMYDATA(obj->data)->characterState.cfg->walkSpeed);
					Box2DBodyApplyForceToCenter(phy->body, force, true);
				}
			}
			return NULL;
		default:
			return NULL;
	}
}
