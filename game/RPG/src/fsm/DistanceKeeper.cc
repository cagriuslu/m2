#include <rpg/fsm/DistanceKeeper.h>
#include <m2/box2d/RayCast.h>
#include <m2/box2d/Detail.h>
#include <m2/Game.h>
#include <m2/Vec2f.h>
#include <initializer_list>

namespace {
	float random_alarm_duration(float recalc_period) {
		return recalc_period / 2.0f + recalc_period * m2::randf() * 1.5f;
	}
}

rpg::DistanceKeeperFsm::DistanceKeeperFsm(const m2::Object* obj, const pb::Ai* ai) : FsmBase(DistanceKeeperMode::Idle), obj(obj), ai(ai) {
	init();
}

std::optional<rpg::DistanceKeeperMode> rpg::DistanceKeeperFsm::handle_signal(const DistanceKeeperFsmSignal& s) {
	if (s.type() == m2::FsmSignalType::EnterState) {
		// Action for EnterState is the same for all states
		arm(random_alarm_duration(ai->recalculation_period()));
		return {};
	}

	switch (state()) {
		case DistanceKeeperMode::Idle:
			switch (s.type()) {
				case m2::FsmSignalType::Alarm: return handle_alarm_while_idle();
				case m2::FsmSignalType::Custom: return {}; // TODO implement small patrol
				default: return {};
			}
		case DistanceKeeperMode::Triggered:
			switch (s.type()) {
				case m2::FsmSignalType::Alarm: return handle_alarm_while_triggered();
				case m2::FsmSignalType::Custom: return handle_physics_step_while_triggered();
				default: return {};
			}
	}
}

std::optional<rpg::DistanceKeeperMode> rpg::DistanceKeeperFsm::handle_alarm_while_idle() {
	// Check if player is close
	if (obj->position.is_near(LEVEL.player()->position, ai->trigger_distance())) {
		escape_towards = find_direction_to_escape();
		if (escape_towards) {
			LOG_TRACE("Escaping towards", *escape_towards);
			return DistanceKeeperMode::Triggered;
		}
	}
	arm(random_alarm_duration(ai->recalculation_period()));
	return {};
}
std::optional<rpg::DistanceKeeperMode> rpg::DistanceKeeperFsm::handle_alarm_while_triggered() {
	// Check if player is still close
	if (obj->position.is_near(LEVEL.player()->position, ai->trigger_distance())) {
		escape_towards = find_direction_to_escape();
		arm(random_alarm_duration(ai->recalculation_period()));
		return {};
	} else {
		escape_towards = {};
		return DistanceKeeperMode::Idle;
	}
}
std::optional<rpg::DistanceKeeperMode> rpg::DistanceKeeperFsm::handle_physics_step_while_triggered() {
	if (escape_towards) {
		m2::Vec2f force = (*escape_towards) * (GAME.delta_time_s() * 30000.0f);
		obj->physique().body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
	}
	return {};
}

std::optional<m2::Vec2f> rpg::DistanceKeeperFsm::find_direction_to_escape() {
	float raycast_length = ai->trigger_distance() / 2.0f;
	auto angle_from_player_to_obj = (obj->position - LEVEL.player()->position).angle_rads();

	auto can_escape = [=](float offset) -> bool {
		auto radians_offset = angle_from_player_to_obj + offset;
		auto raycast_target = obj->position + m2::Vec2f::from_angle(radians_offset).with_length(raycast_length);
		auto raycast_distance = m2::box2d::check_distance(*LEVEL.world, obj->position, raycast_target, m2::box2d::FIXTURE_CATEGORY_OBSTACLE);
		return (raycast_length - raycast_distance) < 0.1f; // 0.1 comparison error
	};

	// Sweep with an increasing angle away from the player
	for (auto sweep_degrees : {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110}) {
		auto sweep_rads = m2::to_radians(sweep_degrees);
		// Try both directions randomly
		auto first_sweep_rads = m2::rand(2u) ? sweep_rads : -sweep_rads;
		auto second_sweep_rads = -first_sweep_rads;
		for (auto offset : {first_sweep_rads, second_sweep_rads}) {
			if (can_escape(offset)) {
				return m2::Vec2f::from_angle(angle_from_player_to_obj + offset);
			}
		}
	}
	return {};
}
