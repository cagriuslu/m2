#include <m2/Automaton.h>

void* AiHitNRunState_Idle(struct _Automaton* am, int signal);
void* AiHitNRunState_Hit(struct _Automaton* am, int signal);
void* AiHitNRunState_Run(struct _Automaton* am, int signal);
void* AiHitNRunState_GaveUp(struct _Automaton* am, int signal);

M2Err AutomatonAiHitNRun_Init(Automaton *am, void* obj, void* phy) {
	M2ERR_REFLECT(Automaton_Init(am));
	am->currState = AiHitNRunState_Idle;
	am->userData_obj = obj;
	am->userData_phy = phy;
	Automaton_ProcessSignal(am, SIG_ENTER);
	return M2OK;
}

void* AiHitNRunState_Idle(struct _Automaton* am, int signal) {
	return NULL;
}
