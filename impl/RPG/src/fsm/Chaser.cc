#include "m2/FSM.h"
#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/Pathfinder.hh"
#include "impl/private/ARPG_Cfg.hh"
#include "../ARPG_Object.hh"
#include <impl/private/object/Enemy.h>
#include <impl/private/fsm/Chaser.h>
#include <m2/M2.h>

#define ALARM_DURATION(recalcPeriod) ((recalcPeriod) / 2.0f + (recalcPeriod) * m2::randf() * 1.5f)

impl::fsm::Chaser::Chaser(m2::Object& obj, const ai::AiBlueprint* blueprint) :
	obj(obj),
	blueprint(blueprint),
	home_position(obj.position),
	target(GAME.objects[GAME.playerId]),
	phy(GAME.physics[obj.physique_id]) {}

void* impl::fsm::Chaser::idle(m2::FSM<Chaser>& automaton, int sig) {
	const auto* blueprint = automaton.data.blueprint;
    switch (sig) {
        case SIG_ENTER:
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
        case SIG_ALARM:
            // Check if player is close
            if (automaton.data.obj.position.distance_sq(automaton.data.target.position) < blueprint->trigger_distance_squared_m) {
                // Check if path exists
                if (PathfinderMap_FindPath(&GAME.pathfinderMap, automaton.data.obj.position, automaton.data.target.position, automaton.data.reverse_waypoints) == M2OK) {
                    return reinterpret_cast<void*>(triggered);
                }
            }
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
        case SIG_AI_PREPHYSICS:
            // TODO implement small patrol
            return nullptr;
        default:
            return nullptr;
    }
}

void AiChase_AttackIfCloseEnough(m2::FSM<impl::fsm::Chaser>& automaton) {
	auto& obj = automaton.data.obj;
	auto& target = automaton.data.target;
	const auto* blueprint = automaton.data.blueprint;
    CharacterState* charState = &(AS_ENEMYDATA(obj.data)->characterState);

    // If player is close enough
    if (obj.position.distance_sq(target.position) < blueprint->attack_distance_squared_m) {
        // Based on what the capability is
        switch (automaton.data.blueprint->capability) {
			case impl::ai::CAPABILITY_RANGED: {
                RangedWeaponState* weaponState = &charState->rangedWeaponState; M2ASSERT(weaponState->cfg);
                // If the weapon cooled down
                if (weaponState->cfg->cooldown_s <= weaponState->cooldownCounter_s) {
                    m2::Object* projectile = &GAME.objects.alloc().first;
                    ObjectProjectile_InitFromCfg(
                            projectile,
                            &weaponState->cfg->projectile,
                            GAME.objects.get_id(&obj),
                            obj.position,
                            target.position - obj.position
                    );
                    // TODO Knockback maybe?
                    weaponState->cooldownCounter_s = 0.0f;
                }
                break;
            }
			case impl::ai::CAPABILITY_MELEE: {
                MeleeWeaponState* weaponState = &charState->meleeWeaponState; M2ASSERT(weaponState->cfg);
                // If the weapon cooled down
                if (weaponState->cfg->cooldown_s <= weaponState->cooldownCounter_s) {
                    m2::Object* melee = &GAME.objects.alloc().first;
                    ObjectMelee_InitFromCfg(
                            melee,
                            &weaponState->cfg->melee,
                            GAME.objects.get_id(&obj),
                            obj.position,
                            target.position - obj.position
                    );
                    weaponState->cooldownCounter_s = 0.0f;
                }
                break;
            }
            case impl::ai::CAPABILITY_EXPLOSIVE: {
                ExplosiveWeaponState* weaponState = &charState->explosiveWeaponState; M2ASSERT(weaponState->cfg);
                // If the weapon cooled down
                if (weaponState->cfg->cooldown_s <= weaponState->cooldownCounter_s) {
                    m2::Object* explosive = &GAME.objects.alloc().first;
                    ObjectExplosive_InitFromCfg(
                            explosive,
                            &weaponState->cfg->explosive,
                            GAME.objects.get_id(&obj),
                            obj.position,
                            target.position - obj.position
                    );
                    // TODO knockback
                    weaponState->cooldownCounter_s = 0.0f;
                }
                break;
            }
            case impl::ai::CAPABILITY_KAMIKAZE: {
                // TODO
                break;
            }
            default:
                break;
        }
    }
}

void* impl::fsm::Chaser::triggered(m2::FSM<Chaser>& automaton, int sig) {
	const auto* blueprint = automaton.data.blueprint;
    auto& player = GAME.objects[GAME.playerId];
    switch (sig) {
        case SIG_ENTER:
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
        case SIG_ALARM: {
            // Check if player is still close
            if (automaton.data.obj.position.distance_sq(player.position) < blueprint->give_up_distance_squared_m) {
                // Recalculate path to player
                PathfinderMap_FindPath(&GAME.pathfinderMap, automaton.data.obj.position, player.position, automaton.data.reverse_waypoints);
            } else {
                // Check if path to homePosition exists
                if (PathfinderMap_FindPath(&GAME.pathfinderMap, automaton.data.obj.position, automaton.data.home_position, automaton.data.reverse_waypoints) == M2OK) {
                    return reinterpret_cast<void*>(gave_up);
                }
            }
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
        }
        case SIG_AI_PREPHYSICS:
            if (1 < automaton.data.reverse_waypoints.size()) {
                auto end = automaton.data.reverse_waypoints.end();
                auto obj_pos = *std::prev(end, 1);
                auto target_pos = *std::prev(end, 2);
                if (obj_pos != target_pos) {
                    auto objPositionF = m2::Vec2f(obj_pos);
                    auto targetPositionF = m2::Vec2f(target_pos);
                    m2::Vec2f direction = (targetPositionF - objPositionF).normalize();
                    m2::Vec2f force = direction * (GAME.deltaTicks_ms * AS_ENEMYDATA(automaton.data.obj.data)->characterState.cfg->walkSpeed);
                    automaton.data.phy.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
                }
            }
            AiChase_AttackIfCloseEnough(automaton);
            return nullptr;
        default:
            return nullptr;
    }
}

void* impl::fsm::Chaser::gave_up(m2::FSM<Chaser>& automaton, int sig) {
	const auto* blueprint = automaton.data.blueprint;
    switch (sig) {
        case SIG_ENTER:
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
        case SIG_EXIT:
            return nullptr;
        case SIG_ALARM:
            // Check if player is close
            if (automaton.data.obj.position.distance_sq(automaton.data.target.position) < automaton.data.blueprint->trigger_distance_squared_m) {
                // Check if path to player exists
                if (PathfinderMap_FindPath(&GAME.pathfinderMap, automaton.data.obj.position, automaton.data.target.position, automaton.data.reverse_waypoints) == M2OK) {
                    return reinterpret_cast<void*>(triggered);
                }
            } else {
                // Check if obj arrived to homePosition
                if (automaton.data.obj.position.distance_sq(automaton.data.home_position) < 1.0f) {
                    return reinterpret_cast<void*>(idle);
                } else {
                    // Recalculate path to homePosition
                    PathfinderMap_FindPath(&GAME.pathfinderMap, automaton.data.obj.position, automaton.data.home_position, automaton.data.reverse_waypoints);
                }
            }
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
        case SIG_AI_PREPHYSICS:
            if (1 < automaton.data.reverse_waypoints.size()) {
                auto end = automaton.data.reverse_waypoints.end();
                auto obj_pos = *std::prev(end, 1);
                auto target_pos = *std::prev(end, 2);
                if (obj_pos != target_pos) {
                    auto objPositionF = m2::Vec2f(obj_pos);
                    auto targetPositionF = m2::Vec2f(target_pos);
                    m2::Vec2f direction = (targetPositionF - objPositionF).normalize();
                    m2::Vec2f force = direction * (GAME.deltaTicks_ms * AS_ENEMYDATA(automaton.data.obj.data)->characterState.cfg->walkSpeed);
                    automaton.data.phy.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
                }
            }
            return nullptr;
        default:
            return nullptr;
    }
}
