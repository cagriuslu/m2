#include "m2/Automaton.hh"
#include <m2/object/Object.hh>
#include "m2/Game.hh"
#include "m2/Pathfinder.hh"
#include <game/ARPG_Cfg.hh>
#include "../ARPG_Object.hh"
#include "AutomatonAiChase.hh"

#define ALARM_DURATION(recalcPeriod) ((recalcPeriod) / 2.0f + (recalcPeriod) * randf() * 1.5f)

void* AutomatonState_AutomatonAiChase_Idle(m2::Automaton* a, int sig);
void* AutomatonState_AutomatonAiChase_Triggered(m2::Automaton* a, int sig);
void* AutomatonState_AutomatonAiChase_GaveUp(m2::Automaton* a, int sig);

game::AutomatonAiChase::AutomatonAiChase(m2::object::Object& obj) : m2::Automaton(AutomatonState_AutomatonAiChase_Idle), obj(obj), phy(GAME.physics[obj.physique_id]) {
    signal(SIG_ENTER);
}

void* AutomatonState_AutomatonAiChase_Idle(m2::Automaton* a, int sig) {
    auto* atm = dynamic_cast<game::AutomatonAiChase*>(a);

    AiState* aiState = &(AS_ENEMYDATA(atm->obj.data)->aiState);
	auto& player = GAME.objects[GAME.playerId];
    switch (sig) {
        case SIG_ENTER:
            atm->arm(ALARM_DURATION(aiState->cfg->recalculationPeriod_s));
            return nullptr;
        case SIG_ALARM:
            // Check if player is close
            if (atm->obj.position.distance_sq(player.position) < aiState->cfg->triggerDistanceSquared_m) {
                // Check if path exists
                if (PathfinderMap_FindPath(&GAME.pathfinderMap, atm->obj.position, player.position, aiState->reversedWaypointList) == M2OK) {
                    return reinterpret_cast<void*>(AutomatonState_AutomatonAiChase_Triggered);
                }
            }
            atm->arm(ALARM_DURATION(aiState->cfg->recalculationPeriod_s));
            return nullptr;
        case SIG_AI_PREPHYSICS:
            // TODO implement small patrol
            return nullptr;
        default:
            return nullptr;
    }
}

void AiChase_AttackIfCloseEnough(m2::object::Object* obj, m2::object::Object* player) {
	CharacterState* charState = &(AS_ENEMYDATA(obj->data)->characterState);
	AiState* aiState = &(AS_ENEMYDATA(obj->data)->aiState);
	M2ASSERT(player);
	// If player is close enough
	if (obj->position.distance_sq(player->position) < aiState->cfg->attackDistanceSquared_m) {
		// Based on what the capability is
		switch (aiState->cfg->capability) {
			case CFG_AI_CAPABILITY_RANGED: {
				RangedWeaponState* weaponState = &charState->rangedWeaponState; M2ASSERT(weaponState->cfg);
				// If the weapon cooled down
				if (weaponState->cfg->cooldown_s <= weaponState->cooldownCounter_s) {
					m2::object::Object* projectile = &GAME.objects.alloc().first;
					ObjectProjectile_InitFromCfg(
						projectile,
						&weaponState->cfg->projectile,
						GAME.objects.get_id(obj),
						obj->position,
						player->position - obj->position
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
					m2::object::Object* melee = &GAME.objects.alloc().first;
					ObjectMelee_InitFromCfg(
						melee,
						&weaponState->cfg->melee,
						GAME.objects.get_id(obj),
						obj->position,
						player->position - obj->position
					);
					weaponState->cooldownCounter_s = 0.0f;
				}
				break;
			}
			case CFG_AI_CAPABILITY_EXPLOSIVE: {
				ExplosiveWeaponState* weaponState = &charState->explosiveWeaponState; M2ASSERT(weaponState->cfg);
				// If the weapon cooled down
				if (weaponState->cfg->cooldown_s <= weaponState->cooldownCounter_s) {
					m2::object::Object* explosive = &GAME.objects.alloc().first;
					ObjectExplosive_InitFromCfg(
						explosive,
						&weaponState->cfg->explosive,
						GAME.objects.get_id(obj),
						obj->position,
						player->position - obj->position
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

void* AutomatonState_AutomatonAiChase_Triggered(m2::Automaton* a, int sig) {
    auto* atm = dynamic_cast<game::AutomatonAiChase*>(a);

	AiState* aiState = &(AS_ENEMYDATA(atm->obj.data)->aiState);
	auto& player = GAME.objects[GAME.playerId];
	switch (sig) {
		case SIG_ENTER:
            atm->arm(ALARM_DURATION(aiState->cfg->recalculationPeriod_s));
			return nullptr;
		case SIG_ALARM: {
			// Check if player is still close
			if (atm->obj.position.distance_sq(player.position) < aiState->cfg->giveUpDistanceSquared_m) {
				// Recalculate path to player
				PathfinderMap_FindPath(&GAME.pathfinderMap, atm->obj.position, player.position, aiState->reversedWaypointList);
			} else {
				// Check if path to homePosition exists
				if (PathfinderMap_FindPath(&GAME.pathfinderMap, atm->obj.position, aiState->homePosition, aiState->reversedWaypointList) == M2OK) {
					return reinterpret_cast<void*>(AutomatonState_AutomatonAiChase_GaveUp);
				}
			}
            atm->arm(ALARM_DURATION(aiState->cfg->recalculationPeriod_s));
			return nullptr;
		}
		case SIG_AI_PREPHYSICS:
			if (1 < aiState->reversedWaypointList.size()) {
                auto end = aiState->reversedWaypointList.end();
                auto obj_pos = *std::prev(end, 1);
                auto target_pos = *std::prev(end, 2);
				if (obj_pos != target_pos) {
					auto objPositionF = m2::vec2f(obj_pos);
					auto targetPositionF = m2::vec2f(target_pos);
					m2::vec2f direction = (targetPositionF - objPositionF).normalize();
					m2::vec2f force = direction * (GAME.deltaTicks_ms * AS_ENEMYDATA(atm->obj.data)->characterState.cfg->walkSpeed);
					atm->phy.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
				}
			}
			AiChase_AttackIfCloseEnough(&atm->obj, &player);
			return nullptr;
		default:
			return nullptr;
	}
}

void* AutomatonState_AutomatonAiChase_GaveUp(m2::Automaton* a, int sig) {
    auto* atm = dynamic_cast<game::AutomatonAiChase*>(a);

	AiState* aiState = &(AS_ENEMYDATA(atm->obj.data)->aiState);
	auto& player = GAME.objects[GAME.playerId];
	switch (sig) {
		case SIG_ENTER:
            atm->arm(ALARM_DURATION(aiState->cfg->recalculationPeriod_s));
			return nullptr;
		case SIG_EXIT:
			return nullptr;
		case SIG_ALARM:
			// Check if player is close
			if (atm->obj.position.distance_sq(player.position) < aiState->cfg->triggerDistanceSquared_m) {
				// Check if path to player exists
				if (PathfinderMap_FindPath(&GAME.pathfinderMap, atm->obj.position, player.position, aiState->reversedWaypointList) == M2OK) {
					return reinterpret_cast<void*>(AutomatonState_AutomatonAiChase_Triggered);
				}
			} else {
				// Check if obj arrived to homePosition
				if (atm->obj.position.distance_sq(aiState->homePosition) < 1.0f) {
					return reinterpret_cast<void*>(AutomatonState_AutomatonAiChase_Idle);
				} else {
					// Recalculate path to homePosition
					PathfinderMap_FindPath(&GAME.pathfinderMap, atm->obj.position, aiState->homePosition, aiState->reversedWaypointList);
				}
			}
            atm->arm(ALARM_DURATION(aiState->cfg->recalculationPeriod_s));
			return nullptr;
		case SIG_AI_PREPHYSICS:
			if (1 < aiState->reversedWaypointList.size()) {
                auto end = aiState->reversedWaypointList.end();
                auto obj_pos = *std::prev(end, 1);
                auto target_pos = *std::prev(end, 2);
				if (obj_pos != target_pos) {
					auto objPositionF = m2::vec2f(obj_pos);
					auto targetPositionF = m2::vec2f(target_pos);
					m2::vec2f direction = (targetPositionF - objPositionF).normalize();
					m2::vec2f force = direction * (GAME.deltaTicks_ms * AS_ENEMYDATA(atm->obj.data)->characterState.cfg->walkSpeed);
					atm->phy.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
				}
			}
			return nullptr;
		default:
			return nullptr;
	}
}
