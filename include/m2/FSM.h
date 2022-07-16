#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <optional>
#include <functional>
#include <cmath>

namespace m2 {
    enum FSMSignal {
        FSMSIG_ENTER = 0,
        FSMSIG_EXIT,
        FSMSIG_ALARM,
        FSMSIG_PREPHY,
        FSMSIG_POSTPHY,
        FSMSIG_PREGFX,
        FSMSIG_POSTGFX,
        FSMSIG_N
    };

    template <typename T>
    struct FSM final {
        using State = void* (*)(FSM<T>& automaton, int sig);

        T data;

        explicit FSM(T&& data) : data(data), current_state(T::initial_state), alarm(NAN) {
            signal(FSMSIG_ENTER);
        }

        void arm(float duration_s) {
            alarm = duration_s;
        }
        void disarm() {
            alarm = NAN;
        }
        void signal(int sig) {
            auto next_state = (*current_state)(*this, sig);
            if (next_state) {
				(*current_state)(*this, FSMSIG_EXIT); // Ignore return value
                current_state = reinterpret_cast<State>(next_state);
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

    private:
        State current_state;
        float alarm;
    };
}

#endif //AUTOMATON_H
