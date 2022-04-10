#include <impl/private/ai/AiBlueprint.h>

std::unique_ptr<impl::ai::AiState> impl::ai::AiBlueprint::get_state() const {
	using namespace type;

	std::unique_ptr<AiState> state;
	if (std::holds_alternative<ChaseBlueprint>(variant)) {
		state = std::make_unique<ChaseState>(this);
	} else if (std::holds_alternative<HitNRunBlueprint>(variant)) {
		state = std::make_unique<HitNRunState>(this);
	} else if (std::holds_alternative<KeepDistanceBlueprint>(variant)) {
		state = std::make_unique<KeepDistanceState>(this);
	} else if (std::holds_alternative<PatrolBlueprint>(variant)) {
		state = std::make_unique<PatrolState>(this);
	}

	return state;
}
