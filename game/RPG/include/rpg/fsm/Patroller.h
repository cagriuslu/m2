#ifndef IMPL_PATROLLER_H
#define IMPL_PATROLLER_H

#include "m2/Object.h"
#include "m2/Fsm.h"

namespace rpg {
	enum class PatrollerMode {
		Idle
	};

	class PatrollerFsmSignal : public m2::FsmSignalBase {
	public:
		using FsmSignalBase::FsmSignalBase;
	};

	class PatrollerFsm : public m2::FsmBase<PatrollerMode, PatrollerFsmSignal> {
	public:
		PatrollerFsm(MAYBE const m2::Object* obj, MAYBE const pb::Ai* ai) : FsmBase(PatrollerMode::Idle) {}

	protected:
		inline std::optional<PatrollerMode> handle_signal(MAYBE const PatrollerFsmSignal& s) override { return {}; }
	};
}

#endif //IMPL_PATROLLER_H
