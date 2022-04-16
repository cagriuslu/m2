#ifndef IMPL_AIBLUEPRINT_H
#define IMPL_AIBLUEPRINT_H

#include "type/Chase.h"
#include "type/HitNRun.h"
#include "type/KeepDistance.h"
#include "type/Patrol.h"
#include <m2/Vec2f.h>
#include <variant>

namespace impl::ai {
    enum Capability {
        CAPABILITY_NONE = 0,
        CAPABILITY_RANGED = 1,
        CAPABILITY_MELEE = 2,
        CAPABILITY_EXPLOSIVE = 4,
        CAPABILITY_KAMIKAZE = 8
    };

    struct AiBlueprint {
        ai::Capability capability; /// Bitmap of capabilities
        float trigger_distance_squared_m; /// Distance AI becomes active
        float attack_distance_squared_m; /// Distance AI is clear to attack player
        float give_up_distance_squared_m; /// Distance AI gives up
        float recalculation_period_s; /// Period after which AI recalculates waypoints (formula: random(s/2, 3s/2))
        std::variant<type::ChaseBlueprint, type::HitNRunBlueprint, type::KeepDistanceBlueprint, type::PatrolBlueprint> variant;
    };

	extern const AiBlueprint chase_000;
}

#endif //IMPL_AIBLUEPRINT_H
