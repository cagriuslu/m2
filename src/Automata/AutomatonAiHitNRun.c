#include "../Automaton.h"

void* AiHitNRunState_Idle(struct _Automaton* am, int signal);
void* AiHitNRunState_Hit(struct _Automaton* am, int signal);
void* AiHitNRunState_Run(struct _Automaton* am, int signal);
void* AiHitNRunState_GaveUp(struct _Automaton* am, int signal);

XErr AutomatonAiHitNRun_Init(Automaton *am, void* obj, void* phy) {
	XERR_REFLECT(Automaton_Init(am));
	am->currState = AiHitNRunState_Idle;
	am->userData_obj = obj;
	am->userData_phy = phy;
	Automaton_ProcessSignal(am, SIG_ENTER);
	return XOK;
}

void* AiHitNRunState_Idle(struct _Automaton* am, int signal) {
	return NULL;
}
