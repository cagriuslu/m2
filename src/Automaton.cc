#include "m2/Automaton.hh"

m2::Automaton::Automaton(State initial_state) : current_state(initial_state), alarm(NAN) {}

void m2::Automaton::arm(float duration) {
    alarm = duration;
}
void m2::Automaton::disarm() {
    alarm = NAN;
}
void m2::Automaton::signal(int sig) {
    auto next_state = current_state(this, sig);
    if (next_state) {
        signal(SIG_EXIT);
        current_state = reinterpret_cast<State>(next_state);
        signal(SIG_ENTER);
    }
}
void m2::Automaton::time(float delta_time) {
    if (not isnan(alarm)) {
        alarm -= delta_time;
        if (alarm <= 0.0f) {
            alarm = NAN;
            this->signal(SIG_ALARM);
        }
    }
}

M2Err Automaton_Init(Automaton* sm) {
	memset(sm, 0, sizeof(Automaton));
	sm->alarm = NAN;
	return M2OK;
}

void Automaton_ArmAlarm(Automaton* sm, float duration) {
	sm->alarm = duration;
}

void Automaton_DisarmAlarm(Automaton* sm) {
	sm->alarm = NAN;
}

void Automaton_ProcessSignal(Automaton* sm, int signal) {
	M2ASSERT(sm && sm->currState);
	void* nextState = (*sm->currState)(sm, signal);
	if (nextState) {
		Automaton_ProcessSignal(sm, SIG_EXIT);
		sm->currState = (AutomatonState)nextState;
		Automaton_ProcessSignal(sm, SIG_ENTER);
	}
}

void Automaton_ProcessTime(Automaton* sm, float deltaTime) {
	if (!isnan(sm->alarm)) {
		sm->alarm -= deltaTime;
		if (sm->alarm < 0.0f) {
			sm->alarm = NAN;
			Automaton_ProcessSignal(sm, SIG_ALARM);
		}
	}
}

void Automaton_Term(Automaton* sm) {
	memset(sm, 0, sizeof(Automaton));
}
