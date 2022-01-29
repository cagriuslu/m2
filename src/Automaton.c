#include "Automaton.h"

XErr Automaton_Init(Automaton* sm) {
	memset(sm, 0, sizeof(Automaton));
	sm->alarm = NAN;
	return XOK;
}

void Automaton_ArmAlarm(Automaton* sm, float duration) {
	sm->alarm = duration;
}

void Automaton_DisarmAlarm(Automaton* sm) {
	sm->alarm = NAN;
}

void Automaton_ProcessSignal(Automaton* sm, int signal) {
	XASSERT(sm && sm->currState);
	void* nextState = (*sm->currState)(sm, signal);
	if (nextState) {
		Automaton_ProcessSignal(sm, SIG_EXIT);
		sm->currState = nextState;
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
