#ifndef IMPL_AIBLUEPRINT_H
#define IMPL_AIBLUEPRINT_H

#include "type/Chase.h"
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

    struct KeepDistanceBlueprint {
        float distance_squared_m; /// Distance AI tries to keep
    };

    struct HitNRunBlueprint {
        float hit_distance_squared_m; /// Distance AI tries to achieve during Hit period
        float hit_duration_s; /// Duration AI stays in Hit period
        float run_distance_squared_m; /// Distance AI tries to achieve during Run period
        float run_duration_s; /// Duration AI stays in Run period

    };

    struct PatrolBlueprint {
        m2::Vec2f area_top_left; /// Top-left of patrol area while idling
        m2::Vec2f area_bottom_right; /// Bottom-right of patrol area while idling
        float speed_mps; /// Patrol speed
    };

    //struct CircleAroundBlueprint;

    struct AiBlueprint {
        ai::Capability capability; /// Bitmap of capabilities
        float trigger_distance_squared_m; /// Distance AI becomes active
        float attack_distance_squared_m; /// Distance AI is clear to attack player
        float give_up_distance_squared_m; /// Distance AI gives up
        float recalculation_period_s; /// Period after which AI recalculates waypoints (formula: random(s/2, 3s/2))
        std::variant<type::ChaseBlueprint, ai::KeepDistanceBlueprint, ai::HitNRunBlueprint, ai::PatrolBlueprint> variant;
    };
}

#endif //IMPL_AIBLUEPRINT_H
