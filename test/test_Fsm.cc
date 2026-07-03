#include <gtest/gtest.h>
#include <m2/common/Fsm.h>

namespace {
	enum class State {
		A,
		B
	};

	/// Trivial signal carrying an integer payload
	class Sig : public m2::FsmSignalBase {
		int _payload{0};
	public:
		using m2::FsmSignalBase::FsmSignalBase;
		Sig(m2::FsmSignalType type, int payload) : m2::FsmSignalBase(type), _payload(payload) {}
		[[nodiscard]] int payload() const { return _payload; }
	};

	/// Concrete FSM that hand-writes its transition logic
	class TestFsm : public m2::FsmBase<State, Sig> {
	public:
		int enter_a_count{0};
		int exit_a_count{0};
		int enter_b_count{0};
		int exit_b_count{0};
		int alarm_fire_count{0};

		TestFsm() { init(State::A); }
		~TestFsm() override { deinit(); }

	protected:
		std::optional<State> HandleSignal(const Sig& s) override {
			switch (state()) {
				case State::A:
					switch (s.type()) {
						case m2::FsmSignalType::EnterState: enter_a_count++; return std::nullopt;
						case m2::FsmSignalType::ExitState: exit_a_count++; return std::nullopt;
						case m2::FsmSignalType::Alarm: alarm_fire_count++; return State::B;
						case m2::FsmSignalType::Custom:
							return s.payload() == 1 ? std::optional<State>{State::B} : std::nullopt;
					}
					break;
				case State::B:
					switch (s.type()) {
						case m2::FsmSignalType::EnterState: enter_b_count++; return std::nullopt;
						case m2::FsmSignalType::ExitState: exit_b_count++; return std::nullopt;
						case m2::FsmSignalType::Custom:
							return s.payload() == 2 ? std::optional<State>{State::A} : std::nullopt;
						case m2::FsmSignalType::Alarm: return std::nullopt;
					}
					break;
			}
			return std::nullopt;
		}
	};

	/// FSM that dispatches through handle_signal_using_handler_map
	class MapFsm : public m2::FsmBase<State, Sig> {
	public:
		int a_custom_calls{0};
		int b_custom_calls{0};

		MapFsm() { init(State::A); }
		~MapFsm() override { deinit(); }

		std::optional<State> OnCustomInA() { a_custom_calls++; return State::B; }
		std::optional<State> OnCustomInB(const Sig&) { b_custom_calls++; return State::A; }

	protected:
		std::optional<State> HandleSignal(const Sig& s) override {
			return m2::handle_signal_using_handler_map<MapFsm, State, Sig>({
					{State::A, m2::FsmSignalType::Custom, &MapFsm::OnCustomInA, nullptr},
					{State::B, m2::FsmSignalType::Custom, nullptr, &MapFsm::OnCustomInB},
			}, *this, s);
		}
	};
}

TEST(FsmBase, InitEntersInitialState) {
	TestFsm fsm;
	EXPECT_EQ(fsm.state(), State::A);
	EXPECT_EQ(fsm.enter_a_count, 1);
	EXPECT_EQ(fsm.exit_a_count, 0);
}

TEST(FsmBase, SignalDrivenTransition) {
	TestFsm fsm;

	// A -> B on Custom payload 1: exits A, enters B
	fsm.signal(Sig{m2::FsmSignalType::Custom, 1});
	EXPECT_EQ(fsm.state(), State::B);
	EXPECT_EQ(fsm.exit_a_count, 1);
	EXPECT_EQ(fsm.enter_b_count, 1);

	// B -> A on Custom payload 2: exits B, re-enters A
	fsm.signal(Sig{m2::FsmSignalType::Custom, 2});
	EXPECT_EQ(fsm.state(), State::A);
	EXPECT_EQ(fsm.exit_b_count, 1);
	EXPECT_EQ(fsm.enter_a_count, 2);
}

TEST(FsmBase, SignalWithoutTransitionKeepsState) {
	TestFsm fsm;
	// Custom payload 99 is not handled as a transition in state A
	fsm.signal(Sig{m2::FsmSignalType::Custom, 99});
	EXPECT_EQ(fsm.state(), State::A);
	EXPECT_EQ(fsm.exit_a_count, 0);
	EXPECT_EQ(fsm.enter_a_count, 1);
}

TEST(FsmBase, AlarmFiresAfterDurationElapses) {
	TestFsm fsm;
	fsm.arm(1.0f);

	// Not enough time passed yet
	fsm.time(0.5f);
	EXPECT_EQ(fsm.state(), State::A);
	EXPECT_EQ(fsm.alarm_fire_count, 0);

	// Cumulative time now exceeds the duration -> alarm fires -> transition to B
	fsm.time(0.6f);
	EXPECT_EQ(fsm.alarm_fire_count, 1);
	EXPECT_EQ(fsm.state(), State::B);
	EXPECT_EQ(fsm.exit_a_count, 1);
	EXPECT_EQ(fsm.enter_b_count, 1);
}

TEST(FsmBase, DisarmedAlarmNeverFires) {
	TestFsm fsm;
	fsm.arm(1.0f);
	fsm.disarm();

	fsm.time(5.0f);
	EXPECT_EQ(fsm.alarm_fire_count, 0);
	EXPECT_EQ(fsm.state(), State::A);
}

TEST(handle_signal_using_handler_map, RoutesToMatchingHandlerAndAppliesNextState) {
	MapFsm fsm;
	EXPECT_EQ(fsm.state(), State::A);

	// Custom in A routes to OnCustomInA, which returns State::B
	fsm.signal(Sig{m2::FsmSignalType::Custom, 0});
	EXPECT_EQ(fsm.a_custom_calls, 1);
	EXPECT_EQ(fsm.state(), State::B);

	// Custom in B routes to OnCustomInB, which returns State::A
	fsm.signal(Sig{m2::FsmSignalType::Custom, 0});
	EXPECT_EQ(fsm.b_custom_calls, 1);
	EXPECT_EQ(fsm.state(), State::A);
}

TEST(handle_signal_using_handler_map, NoMatchingHandlerKeepsState) {
	MapFsm fsm;
	// No table entry for Alarm in state A -> nullopt -> state unchanged, no handler invoked
	fsm.signal(Sig{m2::FsmSignalType::Alarm});
	EXPECT_EQ(fsm.a_custom_calls, 0);
	EXPECT_EQ(fsm.b_custom_calls, 0);
	EXPECT_EQ(fsm.state(), State::A);
}
