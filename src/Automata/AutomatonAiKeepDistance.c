#include "../Automaton.h"

void* AiKeepDistanceState_Idle(struct _Automaton* am, int signal);
void* AiKeepDistanceState_Triggered(struct _Automaton* am, int signal);
void* AiKeepDistanceState_GaveUp(struct _Automaton* am, int signal);

XErr AutomatonAiKeepDistance_Init(Automaton *am, void* obj, void* phy) {
	XERR_REFLECT(Automaton_Init(am));
	am->currState = AiKeepDistanceState_Idle;
	am->userData_obj = obj;
	am->userData_phy = phy;
	Automaton_ProcessSignal(am, SIG_ENTER);
	return XOK;
}

void* AiKeepDistanceState_Idle(struct _Automaton* am, int signal) {
	return NULL;
}
