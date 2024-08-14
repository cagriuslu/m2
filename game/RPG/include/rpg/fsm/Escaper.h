#pragma once
#include "m2/Object.h"
#include "m2/game/Fsm.h"
#include <Enemy.pb.h>

namespace rpg {
	enum class EscaperMode {
		Idle,
		Triggered
	};

	class EscaperFsmSignal : public m2::FsmSignalBase {
		// No content, signifies physics step
	public:
		inline EscaperFsmSignal() : FsmSignalBase(m2::FsmSignalType::Custom) {}
		using FsmSignalBase::FsmSignalBase;
	};

	class EscaperFsm : public m2::FsmBase<EscaperMode, EscaperFsmSignal> {
		m2::Object* obj;
		const pb::Ai* ai;
		std::optional<m2::VecF> escape_towards;

	public:
		EscaperFsm(m2::Object* obj, const pb::Ai* ai);

	protected:
		std::optional<EscaperMode> handle_signal(const EscaperFsmSignal& s) override;

	private:
		std::optional<EscaperMode> handle_alarm_while_idle();
		std::optional<EscaperMode> handle_alarm_while_triggered();
		std::optional<EscaperMode> handle_physics_step_while_triggered();

		std::optional<m2::VecF> find_direction_to_escape();
	};
}
