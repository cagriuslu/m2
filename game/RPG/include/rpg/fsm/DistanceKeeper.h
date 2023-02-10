#ifndef IMPL_DISTANCEKEEPER_H
#define IMPL_DISTANCEKEEPER_H

#include "rpg/ai/AiBlueprint.h"
#include "m2/Object.h"
#include "m2/Fsm.h"

namespace rpg {
	enum class DistanceKeeperMode {
		Idle
	};

	class DistanceKeeperFsmSignal : public m2::FsmSignalBase {
	public:
		using FsmSignalBase::FsmSignalBase;
	};

	class DistanceKeeperFsm : public m2::FsmBase<DistanceKeeperMode, DistanceKeeperFsmSignal> {
	public:
		DistanceKeeperFsm(const m2::Object* obj, const ai::AiBlueprint* blueprint) : FsmBase(DistanceKeeperMode::Idle) {}

	protected:
		inline std::optional<DistanceKeeperMode> handle_signal(const DistanceKeeperFsmSignal& s) override { return {}; }
	};
}

#endif //IMPL_DISTANCEKEEPER_H
