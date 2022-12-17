#include "m2/Fsm.h"
#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/Pathfinder.hh"
#include <rpg/object/Enemy.h>
#include <rpg/object/RangedWeapon.h>
#include <rpg/object/MeleeWeapon.h>
#include <rpg/object/Explosive.h>
#include <rpg/fsm/Chaser.h>
#include <m2/M2.h>

#define ALARM_DURATION(recalcPeriod) ((recalcPeriod) / 2.0f + (recalcPeriod) * m2::randf() * 1.5f)

rpg::ChaserFsmBase::ChaserFsmBase(m2::Object* obj, const ai::AiBlueprint* blueprint) :
	obj(obj),
	blueprint(blueprint),
	home_position(obj->position),
	target(GAME.objects[GAME.playerId]),
	phy(GAME.physics[obj->physique_id()]) {}

void* rpg::ChaserFsmBase::idle(m2::Fsm<ChaserFsmBase>& automaton, int sig) {
	const auto* blueprint = automaton.blueprint;
    switch (sig) {
		case m2::FSM_SIGNAL_ENTER:
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
		case m2::FSM_SIGNAL_ALARM:
            // Check if player is close
            if (automaton.obj->position.distance_sq(automaton.target.position) < blueprint->trigger_distance_squared_m) {
                // Check if path exists
                if (m2::assign_if(PathfinderMap_FindPath(&GAME.pathfinderMap, automaton.obj->position, automaton.target.position), automaton.reverse_waypoints)) {
                    return reinterpret_cast<void*>(triggered);
                }
            }
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
		case rpg::AI_FSM_SIGNAL_PREPHY:
            // TODO implement small patrol
            return nullptr;
        default:
            return nullptr;
    }
}

static void attack_if_close_enough(m2::Fsm<rpg::ChaserFsmBase>& automaton) {
	auto& obj = automaton.obj;
	auto& target = automaton.target;
	const auto* blueprint = automaton.blueprint;
	auto* impl = dynamic_cast<obj::Enemy*>(automaton.obj->impl.get());

    // If player is close enough
    if (obj->position.distance_sq(target.position) < blueprint->attack_distance_squared_m) {
        // Based on what the capability is
        switch (automaton.blueprint->capability) {
			case ai::CAPABILITY_RANGED:
				throw M2ERROR("Chaser ranged weapon not implemented");
			case ai::CAPABILITY_MELEE: {
                if (obj->character().use_item(obj->character().find_items(m2g::pb::ITEM_REUSABLE_SWORD))) {
                    auto& melee = m2::create_object(obj->position, obj->id()).first;
					rpg::create_melee_object(melee, target.position, GAME.get_item(m2g::pb::ITEM_REUSABLE_SWORD), false);
                }
                break;
            }
            case ai::CAPABILITY_EXPLOSIVE: {
				auto& weapon_state = impl->character_state.explosive_weapon_state;
                // If the weapon cooled down
                if (obj->character().use_item(obj->character().find_items(m2g::pb::ITEM_REUSABLE_GRENADE_LAUNCHER))) {
                    auto& explosive = m2::create_object(obj->position, obj->id()).first;
					obj::Explosive::init(
                            explosive,
							&weapon_state->blueprint->explosive,
                            target.position - obj->position
                    );
                    // TODO knockback
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

void* rpg::ChaserFsmBase::triggered(m2::Fsm<ChaserFsmBase>& automaton, int sig) {
	auto* impl = dynamic_cast<obj::Enemy*>(automaton.obj->impl.get());
	const auto* blueprint = automaton.blueprint;
    auto& player = GAME.objects[GAME.playerId];
    switch (sig) {
		case m2::FSM_SIGNAL_ENTER:
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
		case m2::FSM_SIGNAL_ALARM: {
            // Check if player is still close
            if (automaton.obj->position.distance_sq(player.position) < blueprint->give_up_distance_squared_m) {
                // Recalculate path to player
	            m2::assign_if(PathfinderMap_FindPath(&GAME.pathfinderMap, automaton.obj->position, player.position), automaton.reverse_waypoints);
            } else {
                // Check if path to homePosition exists
                if (m2::assign_if(PathfinderMap_FindPath(&GAME.pathfinderMap, automaton.obj->position, automaton.home_position), automaton.reverse_waypoints)) {
                    return reinterpret_cast<void*>(gave_up);
                }
            }
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
        }
		case rpg::AI_FSM_SIGNAL_PREPHY:
			if (not impl->character_state.is_stunned()) {
				if (1 < automaton.reverse_waypoints.size()) {
					auto end = automaton.reverse_waypoints.end();
					auto obj_pos = *std::prev(end, 1);
					auto target_pos = *std::prev(end, 2);
					if (obj_pos != target_pos) {
						auto objPositionF = m2::Vec2f(obj_pos);
						auto targetPositionF = m2::Vec2f(target_pos);
						m2::Vec2f direction = (targetPositionF - objPositionF).normalize();
						m2::Vec2f force = direction * (GAME.deltaTicks_ms * 25.0f);
						automaton.phy.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
					}
				}
				attack_if_close_enough(automaton);
			}
            return nullptr;
        default:
            return nullptr;
    }
}

void* rpg::ChaserFsmBase::gave_up(m2::Fsm<ChaserFsmBase>& automaton, int sig) {
	auto* impl = dynamic_cast<obj::Enemy*>(automaton.obj->impl.get());
	const auto* blueprint = automaton.blueprint;
    switch (sig) {
        case m2::FSM_SIGNAL_ENTER:
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
        case m2::FSM_SIGNAL_ALARM:
            // Check if player is close
            if (automaton.obj->position.distance_sq(automaton.target.position) < automaton.blueprint->trigger_distance_squared_m) {
                // Check if path to player exists
                if (m2::assign_if(PathfinderMap_FindPath(&GAME.pathfinderMap, automaton.obj->position, automaton.target.position), automaton.reverse_waypoints)) {
                    return reinterpret_cast<void*>(triggered);
                }
            } else {
                // Check if obj arrived to homePosition
                if (automaton.obj->position.distance_sq(automaton.home_position) < 1.0f) {
                    return reinterpret_cast<void*>(idle);
                } else {
                    // Recalculate path to homePosition
	                m2::assign_if(PathfinderMap_FindPath(&GAME.pathfinderMap, automaton.obj->position, automaton.home_position), automaton.reverse_waypoints);
                }
            }
            automaton.arm(ALARM_DURATION(blueprint->recalculation_period_s));
            return nullptr;
        case rpg::AI_FSM_SIGNAL_PREPHY:
            if (not impl->character_state.is_stunned() && 1 < automaton.reverse_waypoints.size()) {
                auto end = automaton.reverse_waypoints.end();
                auto obj_pos = *std::prev(end, 1);
                auto target_pos = *std::prev(end, 2);
                if (obj_pos != target_pos) {
                    auto objPositionF = m2::Vec2f(obj_pos);
                    auto targetPositionF = m2::Vec2f(target_pos);
                    m2::Vec2f direction = (targetPositionF - objPositionF).normalize();
                    m2::Vec2f force = direction * (GAME.deltaTicks_ms * 25.0f);
                    automaton.phy.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
                }
            }
            return nullptr;
        default:
            return nullptr;
    }
}
