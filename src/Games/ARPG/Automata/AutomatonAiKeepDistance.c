#include <m2/Automaton.h>

void* AiKeepDistanceState_Idle(struct _Automaton* am, int signal);
void* AiKeepDistanceState_Triggered(struct _Automaton* am, int signal);
void* AiKeepDistanceState_GaveUp(struct _Automaton* am, int signal);

M2Err AutomatonAiKeepDistance_Init(Automaton *am, void* obj, void* phy) {
	M2ERR_REFLECT(Automaton_Init(am));
	am->currState = AiKeepDistanceState_Idle;
	am->userData_obj = obj;
	am->userData_phy = phy;
	Automaton_ProcessSignal(am, SIG_ENTER);
	return M2OK;
}

void* AiKeepDistanceState_Idle(struct _Automaton* am, int signal) {
	return NULL;
}
