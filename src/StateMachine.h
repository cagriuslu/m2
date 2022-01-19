#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "Def.h"

#define SIG_IGNORE()       do { return NULL; } while (0)
#define SIG_GOTO(state) () do { return (state); } while (0)

typedef enum _StateMachineSignal {
	SIG_ENTER = 0,
	SIG_EXIT,
	SIG_ALARM,
	SIG_N
} StateMachineSignal;

struct _StateMachine;
// Return type is StateMachineState*
typedef void* (*StateMachineState)(struct _StateMachine* sm, int signal);

typedef struct _StateMachine {
	StateMachineState currState;
	float alarm;
	const void* userData_cfg;
	void* userData_gfx;
} StateMachine;

XErr StateMachine_Init(StateMachine* sm);
void StateMachine_ArmAlarm(StateMachine* sm, float duration);
void StateMachine_DisarmAlarm(StateMachine* sm);
void StateMachine_ProcessSignal(StateMachine* sm, int signal);
void StateMachine_ProcessTime(StateMachine* sm, float deltaTime);
void StateMachine_Term(StateMachine* sm);

// Specialized state machines

typedef enum _StateMachineCharacterAnimationSignal {
	SIG_CHARANIM_STOP = SIG_N,
	SIG_CHARANIM_WALKDOWN,
	SIG_CHARANIM_WALKRIGHT,
	SIG_CHARANIM_WALKUP,
	SIG_CHARANIM_WALKLEFT,
} StateMachineCharacterAnimationSignal;
XErr StateMachineCharacterAnimation_Init(StateMachine *sm, const void* cfgCharacter, void* gfx);

#endif //STATEMACHINE_H
