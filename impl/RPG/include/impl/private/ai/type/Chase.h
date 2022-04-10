#ifndef IMPL_PRIVATE_AI_TYPE_CHASE_H
#define IMPL_PRIVATE_AI_TYPE_CHASE_H

#include "../AiState.h"

namespace impl::ai::type {
    struct ChaseBlueprint {};

    struct ChaseState : public AiState {
		explicit ChaseState(const AiBlueprint* blueprint);
    };
}

#endif //IMPL_PRIVATE_AI_TYPE_CHASE_H
