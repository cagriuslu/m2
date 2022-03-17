#ifndef AUTOMATON_H
#define AUTOMATON_H

#include "Def.hh"

typedef enum _AutomatonSignal {
	SIG_ENTER = 0,
	SIG_EXIT,
	SIG_ALARM,
	SIG_N
} AutomatonSignal;

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
M2Err AutomatonAiChase_Init(Automaton *am, void* obj, void* phy);
M2Err AutomatonAiKeepDistance_Init(Automaton *am, void* obj, void* phy);
M2Err AutomatonAiHitNRun_Init(Automaton *am, void* obj, void* phy);

#endif //AUTOMATON_H
