#ifndef IMPL_HITNRUNNER_H
#define IMPL_HITNRUNNER_H

#include "rpg/ai/AiBlueprint.h"
#include "m2/Object.h"
#include "m2/Fsm.h"

namespace rpg {
	class HitNRunnerFsmBase {
	public:
		HitNRunnerFsmBase(m2::Object& obj, const ai::AiBlueprint* blueprint);
	};
}

#endif //IMPL_HITNRUNNER_H
