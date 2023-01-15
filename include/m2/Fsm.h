#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <optional>
#include <functional>
#include <cmath>

namespace m2 {
    enum FsmSignal {
        FSM_SIGNAL_EXIT = -3,
        FSM_SIGNAL_ALARM = -2,
		FSM_SIGNAL_ENTER = -1,
		// Custom signals are non-negative
		FSM_SIGNAL_CUSTOM = 0
    };

	/// Provides a specialization for a given FSM base
    template <typename FsmBase>
    class Fsm : public FsmBase {
	public:
		using StateHandler = void* (*)(Fsm<FsmBase>& fsm, int fsm_signal);

	private:
		StateHandler current_state{FsmBase::initial_state};
		float alarm{NAN};

	public:
		template <typename ...Args>
		explicit Fsm(const Args&... args) : FsmBase(args...) {
			signal(FSM_SIGNAL_ENTER);
		}
		template <typename ...Args>
		explicit Fsm(Args&&... args) : FsmBase(args...) {
			signal(FSM_SIGNAL_ENTER);
		}

        void arm(float duration_s) {
            alarm = duration_s;
        }
        void disarm() {
            alarm = NAN;
        }
        void signal(int fsm_signal) {
            auto next_state = (*current_state)(*this, fsm_signal);
            if (next_state) {
				(*current_state)(*this, FSM_SIGNAL_EXIT); // Ignore return value
                current_state = reinterpret_cast<StateHandler>(next_state);
                signal(FSM_SIGNAL_ENTER);
            }
        }
        void time(float delta_time) {
            if (not isnan(alarm)) {
                alarm -= delta_time;
                if (alarm <= 0.0f) {
                    alarm = NAN;
                    signal(FSM_SIGNAL_ALARM);
                }
            }
        }
    };
}

#endif //AUTOMATON_H
