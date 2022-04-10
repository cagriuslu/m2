#ifndef IMPL_KEEPDISTANCE_H
#define IMPL_KEEPDISTANCE_H

#include "../AiState.h"

namespace impl::ai::type {
	struct KeepDistanceBlueprint {
		float distance_squared_m; /// Distance AI tries to keep
	};

	struct KeepDistanceState : public AiState {
		explicit KeepDistanceState(const AiBlueprint* blueprint);
	};
}

#endif //IMPL_KEEPDISTANCE_H
