#include "m2/FSM.h"
#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/Pathfinder.hh"
#include <rpg/object/Enemy.h>
#include <rpg/object/Projectile.h>
#include <rpg/object/Melee.h>
#include <rpg/object/Explosive.h>
#include <rpg/fsm/Chaser.h>
#include <m2/M2.h>

#define ALARM_DURATION(recalcPeriod) ((recalcPeriod) / 2.0f + (recalcPeriod) * m2::randf() * 1.5f)

fsm::Chaser::Chaser(m2::Object& obj, const ai::AiBlueprint* blueprint) :
	obj(obj),
	blueprint(blueprint),
	home_position(obj.position),
	target(GAME.objects[GAME.playerId]),
	phy(GAME.physics[obj.physique_id()]) {}

void* fsm::Chaser::idle(m2::FSM<Chaser>& automaton, int sig) {
	const auto* blueprint = automaton.data.blueprint;
    switch (sig) {
		case m2::FSMSIG_ENTER:
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
		case m2::FSMSIG_ALARM:
            // Check if player is close
            if (automaton.data.obj.position.distance_sq(automaton.data.target.position) < blueprint->trigger_distance_squared_m) {
                // Check if path exists
                if (PathfinderMap_FindPath(&GAME.pathfinderMap, automaton.data.obj.position, automaton.data.target.position, automaton.data.reverse_waypoints) == M2OK) {
                    return reinterpret_cast<void*>(triggered);
                }
            }
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
		case m2::FSMSIG_PREPHY:
            // TODO implement small patrol
            return nullptr;
        default:
            return nullptr;
    }
}

static void attack_if_close_enough(m2::FSM<fsm::Chaser>& automaton) {
	auto& obj = automaton.data.obj;
	auto& target = automaton.data.target;
	const auto* blueprint = automaton.data.blueprint;
	auto* impl = dynamic_cast<obj::Enemy*>(automaton.data.obj.impl.get());

    // If player is close enough
    if (obj.position.distance_sq(target.position) < blueprint->attack_distance_squared_m) {
        // Based on what the capability is
        switch (automaton.data.blueprint->capability) {
			case ai::CAPABILITY_RANGED: {
				auto& weapon_state = impl->character_state.ranged_weapon_state;
                // If the weapon cooled down
                if (weapon_state->blueprint->cooldown_s <= weapon_state->cooldown_counter_s) {
                    auto& projectile = GAME.objects.alloc().first;
					obj::Projectile::init(
                            projectile,
							&weapon_state->blueprint->projectile,
                            GAME.objects.get_id(&obj),
                            obj.position,
                            target.position - obj.position
                    );
                    // TODO Knockback maybe?
					weapon_state->cooldown_counter_s = 0.0f;
                }
                break;
            }
			case ai::CAPABILITY_MELEE: {
				auto& weapon_state = impl->character_state.melee_weapon_state;
                // If the weapon cooled down
                if (weapon_state->blueprint->cooldown_s <= weapon_state->cooldown_counter_s) {
                    auto& melee = GAME.objects.alloc().first;
					obj::Melee::init(
                            melee,
							&weapon_state->blueprint->melee,
                            GAME.objects.get_id(&obj),
                            obj.position,
                            target.position - obj.position
                    );
					weapon_state->cooldown_counter_s = 0.0f;
                }
                break;
            }
            case ai::CAPABILITY_EXPLOSIVE: {
				auto& weapon_state = impl->character_state.explosive_weapon_state;
                // If the weapon cooled down
                if (weapon_state->blueprint->cooldown_s <= weapon_state->cooldown_counter_s) {
                    auto& explosive = GAME.objects.alloc().first;
					obj::Explosive::init(
                            explosive,
							&weapon_state->blueprint->explosive,
                            GAME.objects.get_id(&obj),
                            obj.position,
                            target.position - obj.position
                    );
                    // TODO knockback
					weapon_state->cooldown_counter_s = 0.0f;
                }
                break;
            }
            case ai::CAPABILITY_KAMIKAZE: {
                // TODO
                break;
            }
            default:
                break;
        }
    }
}

void* fsm::Chaser::triggered(m2::FSM<Chaser>& automaton, int sig) {
	auto* impl = dynamic_cast<obj::Enemy*>(automaton.data.obj.impl.get());
	const auto* blueprint = automaton.data.blueprint;
    auto& player = GAME.objects[GAME.playerId];
    switch (sig) {
		case m2::FSMSIG_ENTER:
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
		case m2::FSMSIG_ALARM: {
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
		case m2::FSMSIG_PREPHY:
			if (not impl->character_state.is_stunned()) {
				if (1 < automaton.data.reverse_waypoints.size()) {
					auto end = automaton.data.reverse_waypoints.end();
					auto obj_pos = *std::prev(end, 1);
					auto target_pos = *std::prev(end, 2);
					if (obj_pos != target_pos) {
						auto objPositionF = m2::Vec2f(obj_pos);
						auto targetPositionF = m2::Vec2f(target_pos);
						m2::Vec2f direction = (targetPositionF - objPositionF).normalize();
						m2::Vec2f force = direction * (GAME.deltaTicks_ms * impl->character_state.blueprint->walk_force);
						automaton.data.phy.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
					}
				}
				attack_if_close_enough(automaton);
			}
            return nullptr;
        default:
            return nullptr;
    }
}

void* fsm::Chaser::gave_up(m2::FSM<Chaser>& automaton, int sig) {
	auto* impl = dynamic_cast<obj::Enemy*>(automaton.data.obj.impl.get());
	const auto* blueprint = automaton.data.blueprint;
    switch (sig) {
        case m2::FSMSIG_ENTER:
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
        case m2::FSMSIG_ALARM:
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
        case m2::FSMSIG_PREPHY:
            if (not impl->character_state.is_stunned() && 1 < automaton.data.reverse_waypoints.size()) {
                auto end = automaton.data.reverse_waypoints.end();
                auto obj_pos = *std::prev(end, 1);
                auto target_pos = *std::prev(end, 2);
                if (obj_pos != target_pos) {
                    auto objPositionF = m2::Vec2f(obj_pos);
                    auto targetPositionF = m2::Vec2f(target_pos);
                    m2::Vec2f direction = (targetPositionF - objPositionF).normalize();
                    m2::Vec2f force = direction * (GAME.deltaTicks_ms * impl->character_state.blueprint->walk_force);
                    automaton.data.phy.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
                }
            }
            return nullptr;
        default:
            return nullptr;
    }
}
