#pragma once
#include "m2/Object.h"
#include "m2/game/Fsm.h"

namespace rpg {
	enum class HitNRunnerMode {
		Idle
	};

	class HitNRunnerFsmSignal : public m2::FsmSignalBase {
	public:
		using FsmSignalBase::FsmSignalBase;
	};

	class HitNRunnerFsm : public m2::FsmBase<HitNRunnerMode, HitNRunnerFsmSignal> {
	public:
		HitNRunnerFsm(MAYBE const m2::Object* obj, MAYBE const pb::Ai* ai) : FsmBase() {
			init(HitNRunnerMode::Idle);
		}

	protected:
		inline std::optional<HitNRunnerMode> HandleSignal(MAYBE const HitNRunnerFsmSignal& s) override { return {}; }
	};
}
