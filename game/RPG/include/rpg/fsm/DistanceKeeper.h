#ifndef IMPL_DISTANCEKEEPER_H
#define IMPL_DISTANCEKEEPER_H

#include "m2/Object.h"
#include "m2/Fsm.h"

namespace rpg {
	enum class DistanceKeeperMode {
		Idle,
		Triggered
	};

	class DistanceKeeperFsmSignal : public m2::FsmSignalBase {
		// No content, signifies physics step
	public:
		inline DistanceKeeperFsmSignal() : FsmSignalBase(m2::FsmSignalType::Custom) {}
		using FsmSignalBase::FsmSignalBase;
	};

	class DistanceKeeperFsm : public m2::FsmBase<DistanceKeeperMode, DistanceKeeperFsmSignal> {
		m2::Object* obj;
		const pb::Ai* ai;
		std::optional<m2::Vec2f> escape_towards;

	public:
		DistanceKeeperFsm(m2::Object* obj, const pb::Ai* ai);

	protected:
		std::optional<DistanceKeeperMode> handle_signal(const DistanceKeeperFsmSignal& s) override;

	private:
		std::optional<DistanceKeeperMode> handle_alarm_while_idle();
		std::optional<DistanceKeeperMode> handle_alarm_while_triggered();
		std::optional<DistanceKeeperMode> handle_physics_step_while_triggered();

		std::optional<m2::Vec2f> find_direction_to_escape();
	};
}

#endif //IMPL_DISTANCEKEEPER_H
