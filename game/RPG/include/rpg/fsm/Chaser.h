#ifndef IMPL_CHASE_HH
#define IMPL_CHASE_HH

#include "rpg/ai/type/Chase.h"
#include "m2/Object.h"
#include "m2/Fsm.h"
#include <functional>
#include "rpg/ai/AiBlueprint.h"
#include <list>

namespace rpg {
	enum class ChaserMode {
		Idle,
		Triggered,
		GaveUp
	};

	class ChaserFsmSignal : public m2::FsmSignalBase {
		// No content, signifies physics loop
	public:
		inline ChaserFsmSignal() : FsmSignalBase(m2::FsmSignalType::Custom) {}
		using FsmSignalBase::FsmSignalBase;
	};

	class ChaserFsm : public m2::FsmBase<ChaserMode, ChaserFsmSignal> {
		const m2::Object* obj;
		const ai::AiBlueprint* blueprint;
		const m2::Vec2f home_position;
		std::vector<m2::Vec2i> reverse_waypoints;

	public:
		ChaserFsm(const m2::Object* obj, const ai::AiBlueprint* blueprint);

	protected:
		std::optional<ChaserMode> handle_signal(const ChaserFsmSignal& s) override;

	private:
		std::optional<ChaserMode> handle_alarm_while_idle();
		std::optional<ChaserMode> handle_alarm_while_triggered();
		std::optional<ChaserMode> handle_physics_step_while_triggered();
		std::optional<ChaserMode> handle_alarm_while_gave_up();
		std::optional<ChaserMode> handle_physics_step_while_gave_up();
	};
}

#endif //IMPL_CHASE_HH
