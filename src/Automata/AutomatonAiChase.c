#include "../Automaton.h"

void* AiChaseState_Idle(struct _Automaton* am, int signal);
void* AiChaseState_Triggered(struct _Automaton* am, int signal);
void* AiChaseState_GaveUp(struct _Automaton* am, int signal);

XErr AutomatonAiChase_Init(Automaton *am, void* obj, void* phy) {
	XERR_REFLECT(Automaton_Init(am));
	am->currState = AiChaseState_Idle;
	am->userData_obj = obj;
	am->userData_phy = phy;
	Automaton_ProcessSignal(am, SIG_ENTER);
	return XOK;
}

void* AiChaseState_Idle(struct _Automaton* am, int signal) {
	switch (signal) {
		case SIG_ENTER:
		case SIG_EXIT:
		case SIG_ALARM:
		default:
			return NULL;
	}
}

void* AiChaseState_Triggered(struct _Automaton* am, int signal) {
	switch (signal) {
		case SIG_ENTER:
		case SIG_EXIT:
		case SIG_ALARM:
		default:
			return NULL;
	}
}

void* AiChaseState_GaveUp(struct _Automaton* am, int signal) {
	switch (signal) {
		case SIG_ENTER:
		case SIG_EXIT:
		case SIG_ALARM:
		default:
			return NULL;
	}
}
