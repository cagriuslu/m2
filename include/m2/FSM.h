#ifndef AUTOMATON_H
#define AUTOMATON_H

#include "Def.h"
#include <optional>
#include <functional>

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

enum AutomatonSignal {
	SIG_ENTER = 0,
	SIG_EXIT,
	SIG_ALARM,
	SIG_N
};

struct _Automaton;
/// Return type is again AutomatonState
typedef void* (*AutomatonState)(struct _Automaton* sm, int signal);

typedef struct _Automaton {
	AutomatonState currState;
	float alarm;
	const void* userData_cfg;
	void* userData_obj;
	void* userData_phy;
	void* userData_gfx;
} Automaton;

M2Err Automaton_Init(Automaton* sm);
void Automaton_ArmAlarm(Automaton* sm, float duration);
void Automaton_DisarmAlarm(Automaton* sm);
void Automaton_ProcessSignal(Automaton* sm, int signal);
void Automaton_ProcessTime(Automaton* sm, float deltaTime);
void Automaton_Term(Automaton* sm);

// Specialized automata

typedef enum _AutomatonCharAnimation_Signal {
	SIG_CHARANIM_STOP = SIG_N,
	SIG_CHARANIM_WALKDOWN,
	SIG_CHARANIM_WALKRIGHT,
	SIG_CHARANIM_WALKUP,
	SIG_CHARANIM_WALKLEFT,
} AutomatonCharAnimation_Signal;
M2Err AutomatonCharAnimation_Init(Automaton *sm, const void* cfgCharacter, void* gfx);

typedef enum _AutomatonAi_Signal {
	SIG_AI_PREPHYSICS = SIG_N,
} AutomatonAi_Signal;
M2Err AutomatonAiKeepDistance_Init(Automaton *am, void* obj, void* phy);
M2Err AutomatonAiHitNRun_Init(Automaton *am, void* obj, void* phy);

#endif //AUTOMATON_H
