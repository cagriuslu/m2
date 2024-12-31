#pragma once
#include "m2/Object.h"
#include "m2/game/Fsm.h"

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
		PatrollerFsm(MAYBE const m2::Object* obj, MAYBE const pb::Ai* ai) : FsmBase() {
			init(PatrollerMode::Idle);
		}

	protected:
		inline std::optional<PatrollerMode> HandleSignal(MAYBE const PatrollerFsmSignal& s) override { return {}; }
	};
}
