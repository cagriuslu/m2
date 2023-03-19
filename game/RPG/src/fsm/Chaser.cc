#include "m2/Fsm.h"
#include <m2/Object.h>
#include "m2/Game.h"
#include "m2/Pathfinder.h"
#include <rpg/object/Enemy.h>
#include <rpg/object/RangedWeapon.h>
#include <rpg/object/MeleeWeapon.h>
#include <rpg/fsm/Chaser.h>
#include <m2/M2.h>

namespace {
	float random_alarm_duration(float recalc_period) {
		return recalc_period / 2.0f + recalc_period * m2::randf() * 1.5f;
	}
}

rpg::ChaserFsm::ChaserFsm(const m2::Object* obj, const ai::AiBlueprint* blueprint) : FsmBase(ChaserMode::Idle), obj(obj), blueprint(blueprint), home_position(obj->position) {
	init();
}

std::optional<rpg::ChaserMode> rpg::ChaserFsm::handle_signal(const ChaserFsmSignal &s) {
	if (s.type() == m2::FsmSignalType::EnterState) {
		// Action for EnterState is the same for all states
		arm(random_alarm_duration(blueprint->recalculation_period_s));
		return {};
	}

	switch (state()) {
		case ChaserMode::Idle:
			switch (s.type()) {
				case m2::FsmSignalType::Alarm: return handle_alarm_while_idle();
				case m2::FsmSignalType::Custom: return {}; // TODO implement small patrol
				default: return {};
			}
		case ChaserMode::Triggered:
			switch (s.type()) {
				case m2::FsmSignalType::Alarm: return handle_alarm_while_triggered();
				case m2::FsmSignalType::Custom: return handle_physics_step_while_triggered();
				default: return {};
			}
		case ChaserMode::GaveUp:
			switch (s.type()) {
				case m2::FsmSignalType::Alarm: return handle_alarm_while_gave_up();
				case m2::FsmSignalType::Custom: return handle_physics_step_while_gave_up();
				default: return {};
			}
	}
}

std::optional<rpg::ChaserMode> rpg::ChaserFsm::handle_alarm_while_idle() {
	// Check if player is close
	if (obj->position.is_near(LEVEL.player()->position, blueprint->trigger_distance_m)) {
		// Check if path exists
		auto smooth_path = LEVEL.pathfinder->find_smooth_path(obj->position, LEVEL.player()->position, blueprint->give_up_distance_m);
		if (not smooth_path.empty()) {
			reverse_waypoints = std::move(smooth_path);
			return ChaserMode::Triggered;
		}
	} else {
		arm(random_alarm_duration(blueprint->recalculation_period_s));
		return {};
	}
}

std::optional<rpg::ChaserMode> rpg::ChaserFsm::handle_alarm_while_triggered() {
	// Check if player is still close
	if (obj->position.is_near(LEVEL.player()->position, blueprint->give_up_distance_m)) {
		// Recalculate path to player
		auto smooth_path = LEVEL.pathfinder->find_smooth_path(obj->position, LEVEL.player()->position, blueprint->give_up_distance_m);
		if (not smooth_path.empty()) {
			reverse_waypoints = std::move(smooth_path);
		}
	} else {
		// Check if path to homePosition exists
		auto smooth_path = LEVEL.pathfinder->find_smooth_path(obj->position, LEVEL.player()->position, blueprint->give_up_distance_m);
		if (not smooth_path.empty()) {
			reverse_waypoints = std::move(smooth_path);
			return ChaserMode::GaveUp;
		}
	}
	arm(random_alarm_duration(blueprint->recalculation_period_s));
	return {};
}

std::optional<rpg::ChaserMode> rpg::ChaserFsm::handle_physics_step_while_triggered() {
	// If not stunned
	if (not obj->character().has_resource(m2g::pb::RESOURCE_STUN_TTL)) {
		if (1 < reverse_waypoints.size()) {
			auto end = reverse_waypoints.end();
			auto obj_pos = *std::prev(end, 1);
			auto target_pos = *std::prev(end, 2);
			if (obj_pos != target_pos) {
				auto objPositionF = m2::Vec2f(obj_pos);
				auto targetPositionF = m2::Vec2f(target_pos);
				m2::Vec2f direction = (targetPositionF - objPositionF).normalize();
				m2::Vec2f force = direction * (GAME.deltaTime_s * 25000.0f);
				obj->physique().body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
			}
		}
		// Attack if player is close
		if (obj->position.is_near(LEVEL.player()->position, blueprint->attack_distance_m)) {
			// Based on what the capability is
			switch (blueprint->capability) {
				case ai::CAPABILITY_RANGED:
					throw M2ERROR("Chaser ranged weapon not implemented");
				case ai::CAPABILITY_MELEE:
					if (obj->character().use_item(obj->character().find_items(m2g::pb::ITEM_REUSABLE_SWORD))) {
						auto& melee = m2::create_object(obj->position, obj->id()).first;
						rpg::create_melee_object(melee, LEVEL.player()->position, *GAME.get_item(m2g::pb::ITEM_REUSABLE_SWORD), false);
					}
					break;
				case ai::CAPABILITY_EXPLOSIVE:
					throw M2ERROR("Chaser explosive weapon not implemented");
				case ai::CAPABILITY_KAMIKAZE:
					throw M2ERROR("Chaser kamikaze not implemented");
				default:
					break;
			}
		}
	}
	return {};
}

std::optional<rpg::ChaserMode> rpg::ChaserFsm::handle_alarm_while_gave_up() {
	// Check if player is close
	if (obj->position.is_near(LEVEL.player()->position, blueprint->trigger_distance_m)) {
		// Check if path to player exists
		auto smooth_path = LEVEL.pathfinder->find_smooth_path(obj->position, LEVEL.player()->position, blueprint->give_up_distance_m);
		if (not smooth_path.empty()) {
			reverse_waypoints = std::move(smooth_path);
			return ChaserMode::Triggered;
		}
	} else {
		// Check if obj arrived to homePosition
		if (obj->position.is_near(home_position, 1.0f)) {
			return ChaserMode::Idle;
		} else {
			// Recalculate path to homePosition
			auto smooth_path = LEVEL.pathfinder->find_smooth_path(obj->position, LEVEL.player()->position, blueprint->give_up_distance_m);
			if (not smooth_path.empty()) {
				reverse_waypoints = std::move(smooth_path);
			}
		}
	}
	arm(random_alarm_duration(blueprint->recalculation_period_s));
	return {};
}

std::optional<rpg::ChaserMode> rpg::ChaserFsm::handle_physics_step_while_gave_up() {
	// If not stunned
	if (not obj->character().has_resource(m2g::pb::RESOURCE_STUN_TTL) && 1 < reverse_waypoints.size()) {
		auto end = reverse_waypoints.end();
		auto obj_pos = *std::prev(end, 1);
		auto target_pos = *std::prev(end, 2);
		if (obj_pos != target_pos) {
			auto objPositionF = m2::Vec2f(obj_pos);
			auto targetPositionF = m2::Vec2f(target_pos);
			m2::Vec2f direction = (targetPositionF - objPositionF).normalize();
			m2::Vec2f force = direction * (GAME.deltaTime_s * 25000.0f);
			obj->physique().body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
		}
	}
	return {};
}
