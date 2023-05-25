#ifndef IMPL_CHASE_HH
#define IMPL_CHASE_HH

#include "m2/Object.h"
#include "m2/Pathfinder.h"
#include "m2/Fsm.h"
#include <Enemy.pb.h>
#include <list>
#include <functional>

namespace rpg {
	enum class ChaserMode {
		Idle,
		Triggered,
		GaveUp
	};

	class ChaserFsmSignal : public m2::FsmSignalBase {
	public:
		enum class Type {
			PHY_STEP,
			GOT_HIT,
		};
	private:
		Type _type;
	public:
		inline explicit ChaserFsmSignal(Type type) : FsmSignalBase(m2::FsmSignalType::Custom), _type(type) {}
		using FsmSignalBase::FsmSignalBase;

		inline bool got_hit() const { return _type == Type::GOT_HIT; }
		inline bool phy_step() const { return _type == Type::PHY_STEP; }
	};

	class ChaserFsm : public m2::FsmBase<ChaserMode, ChaserFsmSignal> {
		m2::Object* obj;
		const pb::Ai* ai;
		m2::Vec2f home_position;
		m2::Path reverse_path;

	public:
		ChaserFsm(m2::Object* obj, const pb::Ai* ai);

	protected:
		std::optional<ChaserMode> handle_signal(const ChaserFsmSignal& s) override;

	private:
		std::optional<ChaserMode> handle_alarm_while_idle();
		std::optional<ChaserMode> handle_hit_while_idle_or_given_up();
		std::optional<ChaserMode> handle_alarm_while_triggered();
		std::optional<ChaserMode> handle_physics_step_while_triggered();
		std::optional<ChaserMode> handle_alarm_while_gave_up();
		std::optional<ChaserMode> handle_physics_step_while_gave_up();

		bool find_path(const m2::Vec2f& target, float max_distance);
		void follow_waypoints();
	};
}

#endif //IMPL_CHASE_HH
