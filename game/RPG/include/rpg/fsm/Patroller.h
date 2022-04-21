#ifndef IMPL_PATROLLER_H
#define IMPL_PATROLLER_H

#include "rpg/ai/AiBlueprint.h"
#include "m2/Object.h"
#include "m2/FSM.h"

namespace impl::fsm {
	class Patroller {
	public:
		Patroller(m2::Object& obj, const ai::AiBlueprint* blueprint);
	};
}

#endif //IMPL_PATROLLER_H
