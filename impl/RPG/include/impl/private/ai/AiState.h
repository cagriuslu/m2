#ifndef IMPL_AI_AISTATE_H
#define IMPL_AI_AISTATE_H

#include <m2/Vec2f.h>
#include <list>

namespace impl::ai {
	struct AiBlueprint;

    struct AiState {
        const AiBlueprint* blueprint;
        m2::Vec2f home_position;
        std::list<m2::Vec2i> reverse_waypoints;

		explicit AiState(const AiBlueprint* blueprint);
    };
}

#endif //IMPL_AI_AISTATE_H
