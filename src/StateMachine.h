#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "Def.h"

#define SIG_IGNORE()       do { return NULL; } while (0)
#define SIG_GOTO(state) () do { return (state); } while (0)

typedef enum _StateMachineSignal {
	SIG_INIT = 0,
	SIG_ENTER,
	SIG_EXIT,
	SIG_N
} StateMachineSignal;

struct _StateMachine;
// Return type is StateMachineState*
typedef void* (*StateMachineState)(struct _StateMachine* sm, int signal);

typedef struct _StateMachine {
	StateMachineState currState;
	float alarm;
	void *userData;
} StateMachine;

XErr StateMachine_Init(StateMachine* sm, StateMachineState initialState);
void StateMachine_ArmAlarm(StateMachine* sm, float duration);
void StateMachine_ProcessSignal(StateMachine* sm, int signal);
void StateMachine_ProcessTime(StateMachine* sm, float deltaTime);
void StateMachine_Term(StateMachine* sm);

// Specialized state machines
XErr StateMachineCharacterAnimation_Init(StateMachine *sm);

#endif //STATEMACHINE_H
