#ifndef RPG_AI_SIGNAL_H
#define RPG_AI_SIGNAL_H

#include <m2/Fsm.h>

namespace rpg {
	enum AiFsmSignal {
		AI_FSM_SIGNAL_PREPHY = m2::FSM_SIGNAL_CUSTOM
	};
}

#endif //RPG_AI_SIGNAL_H
