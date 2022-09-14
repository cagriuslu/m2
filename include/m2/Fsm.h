#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <optional>
#include <functional>
#include <cmath>

namespace m2 {
    enum FsmSignal : unsigned {
        FSMSIG_ENTER = 0,
        FSMSIG_EXIT,
        FSMSIG_ALARM,
        FSMSIG_PREPHY,
        FSMSIG_POSTPHY,
        FSMSIG_PREGFX,
        FSMSIG_POSTGFX,
        FSMSIG_N
    };

	using FsmStateHandler = void*;

    template <typename Data>
    class Fsm {
	public:
		using StateHandler = FsmStateHandler (*)(Fsm<Data>& automaton, unsigned sig);

	private:
		StateHandler current_state;
		float alarm;

	public:
        Data data;

        explicit Fsm(Data&& data) : current_state(Data::initial_state), alarm(NAN), data(data) {
            signal(FSMSIG_ENTER);
        }

        void arm(float duration_s) {
            alarm = duration_s;
        }
        void disarm() {
            alarm = NAN;
        }
        void signal(unsigned sig) {
            auto next_state = (*current_state)(*this, sig);
            if (next_state) {
				(*current_state)(*this, FSMSIG_EXIT); // Ignore return value
                current_state = reinterpret_cast<StateHandler>(next_state);
                signal(FSMSIG_ENTER);
            }
        }
        void time(float delta_time) {
            if (not isnan(alarm)) {
                alarm -= delta_time;
                if (alarm <= 0.0f) {
                    alarm = NAN;
                    signal(FSMSIG_ALARM);
                }
            }
        }
    };
}

#endif //AUTOMATON_H
