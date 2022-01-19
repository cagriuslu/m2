#include "StateMachine.h"

XErr StateMachine_Init(StateMachine* sm) {
	memset(sm, 0, sizeof(StateMachine));
	sm->alarm = NAN;
	return XOK;
}

void StateMachine_ArmAlarm(StateMachine* sm, float duration) {
	sm->alarm = duration;
}

void StateMachine_DisarmAlarm(StateMachine* sm) {
	sm->alarm = NAN;
}

void StateMachine_ProcessSignal(StateMachine* sm, int signal) {
	XASSERT(sm && sm->currState);
	void* nextState = (*sm->currState)(sm, signal);
	if (nextState) {
		StateMachine_ProcessSignal(sm, SIG_EXIT);
		sm->currState = nextState;
		StateMachine_ProcessSignal(sm, SIG_ENTER);
	}
}

void StateMachine_ProcessTime(StateMachine* sm, float deltaTime) {
	if (!isnan(sm->alarm)) {
		sm->alarm -= deltaTime;
		if (sm->alarm < 0.0f) {
			sm->alarm = NAN;
			StateMachine_ProcessSignal(sm, SIG_ALARM);
		}
	}
}

void StateMachine_Term(StateMachine* sm) {
	memset(sm, 0, sizeof(StateMachine));
}
