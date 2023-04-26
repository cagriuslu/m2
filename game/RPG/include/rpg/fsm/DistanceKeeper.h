#ifndef IMPL_DISTANCEKEEPER_H
#define IMPL_DISTANCEKEEPER_H

#include "m2/Object.h"
#include "m2/Fsm.h"

namespace rpg {
	enum class DistanceKeeperMode {
		Idle,
		Triggered,
		GaveUp
	};

	class DistanceKeeperFsmSignal : public m2::FsmSignalBase {
		// No content, signifies physics step
	public:
		inline DistanceKeeperFsmSignal() : FsmSignalBase(m2::FsmSignalType::Custom) {}
		using FsmSignalBase::FsmSignalBase;
	};

	class DistanceKeeperFsm : public m2::FsmBase<DistanceKeeperMode, DistanceKeeperFsmSignal> {
		const m2::Object* obj;
		const pb::Ai* ai;
		m2::Vec2f home_position;

	public:
		DistanceKeeperFsm(const m2::Object* obj, const pb::Ai* ai);

	protected:
		std::optional<DistanceKeeperMode> handle_signal(const DistanceKeeperFsmSignal& s) override;
	};
}

#endif //IMPL_DISTANCEKEEPER_H
