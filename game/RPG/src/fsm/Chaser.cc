#include <rpg/fsm/Chaser.h>
#include <rpg/object/Enemy.h>
#include <m2/Game.h>

namespace {
	float random_alarm_duration(float recalc_period) {
		return recalc_period / 2.0f + recalc_period * m2::randf() * 1.5f;
	}

	m2::Path::const_reverse_iterator reverse_find_first_local_min(const m2::Path& list, const m2::VecF& pos) {
		auto closest_it = list.crbegin();
		auto closest_distance_sq = pos.distance_sq(*closest_it);
		for (auto it = std::next(closest_it); it != list.rend(); ++it) {
			auto new_closest_distance_sq = pos.distance_sq(*it);
			if (closest_distance_sq < new_closest_distance_sq) {
				break;
			} else {
				closest_it = it;
				closest_distance_sq = new_closest_distance_sq;
			}
		}
		return closest_it;
	}
}

rpg::ChaserFsm::ChaserFsm(m2::Object* obj, const pb::Ai* ai) : FsmBase(ChaserMode::Idle), obj(obj), ai(ai), home_position(obj->position) {
	init();
}

std::optional<rpg::ChaserMode> rpg::ChaserFsm::handle_signal(const ChaserFsmSignal &s) {
	if (s.type() == m2::FsmSignalType::EnterState) {
		// Action for EnterState is the same for all states
		arm(random_alarm_duration(ai->recalculation_period()));
		return {};
	}

	switch (state()) {
		case ChaserMode::Idle:
			switch (s.type()) {
				case m2::FsmSignalType::Alarm: return handle_alarm_while_idle();
				case m2::FsmSignalType::Custom:
					if (s.got_hit()) {
						return handle_hit_while_idle_or_given_up();
					} else {
						return {}; // TODO implement small patrol
					}
				default: return {};
			}
		case ChaserMode::Triggered:
			switch (s.type()) {
				case m2::FsmSignalType::Alarm: return handle_alarm_while_triggered();
				case m2::FsmSignalType::Custom:
					if (s.phy_step()) {
						return handle_physics_step_while_triggered();
					} else {
						return {};
					}
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
	if (obj->position.is_near(LEVEL.player()->position, ai->trigger_distance())) {
		// Check if path exists
		if (find_path(LEVEL.player()->position, ai->chaser().give_up_distance())) {
			return ChaserMode::Triggered;
		}
	}
	arm(random_alarm_duration(ai->recalculation_period()));
	return {};
}

std::optional<rpg::ChaserMode> rpg::ChaserFsm::handle_hit_while_idle_or_given_up() {
	// Check if path exists
	if (find_path(LEVEL.player()->position, ai->hit_trigger_distance())) {
		return ChaserMode::Triggered;
	}
	return {};
}

std::optional<rpg::ChaserMode> rpg::ChaserFsm::handle_alarm_while_triggered() {
	// Check if player is still close
	if (obj->position.is_near(LEVEL.player()->position, ai->chaser().give_up_distance())) {
		// Recalculate path to player
		find_path(LEVEL.player()->position, ai->chaser().give_up_distance());
	} else {
		// Check if path to home_position exists
		if (find_path(home_position, INFINITY)) {
			return ChaserMode::GaveUp;
		}
	}
	arm(random_alarm_duration(ai->recalculation_period()));
	return {};
}

std::optional<rpg::ChaserMode> rpg::ChaserFsm::handle_physics_step_while_triggered() {
	follow_waypoints();
	Enemy::attack_if_close(*obj, *ai);
	return {};
}

std::optional<rpg::ChaserMode> rpg::ChaserFsm::handle_alarm_while_gave_up() {
	// Check if player is close
	if (obj->position.is_near(LEVEL.player()->position, ai->trigger_distance())) {
		// Check if path to player exists
		if (find_path(LEVEL.player()->position, ai->chaser().give_up_distance())) {
			return ChaserMode::Triggered;
		}
	} else {
		// Check if obj arrived to homePosition
		if (obj->position.is_near(home_position, 1.0f)) {
			return ChaserMode::Idle;
		} else {
			// Recalculate path to homePosition
			find_path(home_position, INFINITY);
		}
	}
	arm(random_alarm_duration(ai->recalculation_period()));
	return {};
}

std::optional<rpg::ChaserMode> rpg::ChaserFsm::handle_physics_step_while_gave_up() {
	follow_waypoints();
	return {};
}

bool rpg::ChaserFsm::find_path(const m2::VecF& target, float max_distance) {
	auto smooth_path = LEVEL.pathfinder->find_grid_path(obj->position, target, max_distance);
	if (not smooth_path.empty()) {
		_reverse_path = std::move(smooth_path);
		return true;
	}
	return false;
}

void rpg::ChaserFsm::follow_waypoints() {
	auto closest_waypoint_it = reverse_find_first_local_min(_reverse_path, obj->position);
	auto next_waypoint_it = std::next(closest_waypoint_it);
	if (next_waypoint_it != _reverse_path.crend()) {
		auto target = m2::VecF{*next_waypoint_it};
		Enemy::move_towards(*obj, target - obj->position, 30000.0f);
	} else {
		// Player is very close
	}
}
