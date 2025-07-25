#include <rpg/fsm/Escaper.h>
#include <m2/box2d/RayCast.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/math/VecF.h>
#include <m2/third_party/physics/ColliderCategory.h>
#include <rpg/object/Enemy.h>

namespace {
	float random_alarm_duration(float recalc_period) {
		return recalc_period / 2.0f + recalc_period * m2::RandomF() * 1.5f;
	}
}

rpg::EscaperFsm::EscaperFsm(m2::Object* obj, const pb::Ai* ai) : FsmBase(), obj(obj), ai(ai) {
	init(EscaperMode::Idle);
}

std::optional<rpg::EscaperMode> rpg::EscaperFsm::HandleSignal(const EscaperFsmSignal& s) {
	if (s.type() == m2::FsmSignalType::EnterState) {
		// UiAction for EnterState is the same for all states
		arm(random_alarm_duration(ai->recalculation_period()));
		return {};
	}

	switch (state()) {
		case EscaperMode::Idle:
			switch (s.type()) {
				case m2::FsmSignalType::Alarm: return handle_alarm_while_idle();
				case m2::FsmSignalType::Custom: return {}; // TODO implement small patrol
				default: return {};
			}
		case EscaperMode::Triggered:
			switch (s.type()) {
				case m2::FsmSignalType::Alarm: return handle_alarm_while_triggered();
				case m2::FsmSignalType::Custom: return handle_physics_step_while_triggered();
				default: return {};
			}
		default:
			return {};
	}
}

std::optional<rpg::EscaperMode> rpg::EscaperFsm::handle_alarm_while_idle() {
	// Check if player is close
	if (obj->position.IsNear(M2_PLAYER.position, ai->trigger_distance())) {
		escape_towards = find_direction_to_escape();
		if (escape_towards) {
			LOG_TRACE("Escaping towards", *escape_towards);
			return EscaperMode::Triggered;
		}
	}
	arm(random_alarm_duration(ai->recalculation_period()));
	return {};
}
std::optional<rpg::EscaperMode> rpg::EscaperFsm::handle_alarm_while_triggered() {
	// Check if player is still close
	if (obj->position.IsNear(M2_PLAYER.position, ai->trigger_distance())) {
		escape_towards = find_direction_to_escape();
		arm(random_alarm_duration(ai->recalculation_period()));
		return {};
	} else {
		escape_towards = {};
		return EscaperMode::Idle;
	}
}
std::optional<rpg::EscaperMode> rpg::EscaperFsm::handle_physics_step_while_triggered() {
	if (escape_towards) {
		Enemy::move_towards(*obj, *escape_towards, 30000.0f);
	}
	Enemy::attack_if_close(*obj, *ai);
	return {};
}

std::optional<m2::VecF> rpg::EscaperFsm::find_direction_to_escape() {
	float raycast_length = ai->trigger_distance() / 2.0f;
	auto angle_from_player_to_obj = (obj->position - M2_PLAYER.position).GetAngle();

	auto can_escape = [=, this](float offset) -> bool {
		auto radians_offset = angle_from_player_to_obj + offset;
		auto raycast_target = obj->position + m2::VecF::CreateUnitVectorWithAngle(radians_offset).WithLength(raycast_length);
		auto raycast_distance = m2::box2d::CheckDistance(*M2_LEVEL.world[I(m2::PhysicsLayer::P0)], obj->position, raycast_target, m2::third_party::physics::gColliderCategoryToParams[m2::I(m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_OBSTACLE)].belongsTo);
		return (raycast_length - raycast_distance) < 0.1f; // 0.1 comparison error
	};

	// Sweep with an increasing angle away from the player
	for (auto sweep_degrees : {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110}) {
		auto sweep_rads = m2::ToRadians(sweep_degrees);
		// Try both directions randomly
		auto first_sweep_rads = m2::Random(2u) ? sweep_rads : -sweep_rads;
		auto second_sweep_rads = -first_sweep_rads;
		for (auto offset : {first_sweep_rads, second_sweep_rads}) {
			if (can_escape(offset)) {
				return m2::VecF::CreateUnitVectorWithAngle(angle_from_player_to_obj + offset);
			}
		}
	}
	return {};
}
