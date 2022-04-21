#ifndef IMPL_PATROL_H
#define IMPL_PATROL_H

#include "m2/Vec2f.h"

namespace ai::type {
	struct PatrolBlueprint {
		m2::Vec2f area_top_left; /// Top-left of patrol area while idling
		m2::Vec2f area_bottom_right; /// Bottom-right of patrol area while idling
		float speed_mps; /// Patrol speed
	};
}

#endif //IMPL_PATROL_H
