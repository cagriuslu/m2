#ifndef IMPL_AI_AISTATE_H
#define IMPL_AI_AISTATE_H

#include "AiBlueprint.h"
#include <list>

namespace impl::ai {
    struct AiState {
        const AiBlueprint* blueprint;
        m2::Vec2f home_position;
        std::list<m2::Vec2i> reverse_waypoints;
    };
}

#endif //IMPL_AI_AISTATE_H
