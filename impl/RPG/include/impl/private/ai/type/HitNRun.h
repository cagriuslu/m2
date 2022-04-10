#ifndef IMPL_HITNRUN_H
#define IMPL_HITNRUN_H

#include "../AiState.h"

namespace impl::ai::type {
	struct HitNRunBlueprint {
		float hit_distance_squared_m; /// Distance AI tries to achieve during Hit period
		float hit_duration_s; /// Duration AI stays in Hit period
		float run_distance_squared_m; /// Distance AI tries to achieve during Run period
		float run_duration_s; /// Duration AI stays in Run period
	};

	struct HitNRunState : public AiState {
		explicit HitNRunState(const AiBlueprint* blueprint);
	};
}

#endif //IMPL_HITNRUN_H
