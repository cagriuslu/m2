#ifndef IMPL_DISTANCEKEEPER_H
#define IMPL_DISTANCEKEEPER_H

#include "rpg/ai/AiBlueprint.h"
#include "m2/Object.h"
#include "m2/Fsm.h"

namespace rpg {
	class DistanceKeeperFsmBase {
	public:
		DistanceKeeperFsmBase(m2::Object& obj, const ai::AiBlueprint* blueprint);
	};
}

#endif //IMPL_DISTANCEKEEPER_H
