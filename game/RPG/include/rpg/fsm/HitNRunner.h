#ifndef IMPL_HITNRUNNER_H
#define IMPL_HITNRUNNER_H

#include "rpg/ai/AiBlueprint.h"
#include "m2/Object.h"
#include "m2/Fsm.h"

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
		HitNRunnerFsm(const m2::Object* obj, const ai::AiBlueprint* blueprint) : FsmBase(HitNRunnerMode::Idle) {}

	protected:
		inline std::optional<HitNRunnerMode> handle_signal(const HitNRunnerFsmSignal& s) override { return {}; }
	};
}

#endif //IMPL_HITNRUNNER_H
