#include <m2/multi_player/lockstep/LevelReplayer.h>
#include <m2/ProxyHelper.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

bool LevelReplayer::IsFinished() const {
	// TODO
	return false;
}

std::optional<LevelReplayer::SimulationInputs> LevelReplayer::GetNextSimulationInputs() {
	if (_physicsSimulationsCounter == m2g::LOCKSTEP_PHYSICS_SIMULATION_COUNT_PER_GAME_TICK) {
		SimulationInputs inputs; // TODO fetch from db
		LOG_NETWORK("Received inputs to simulate");
		_physicsSimulationsCounter = 0;
		LOG_NETWORK("Simulation inputs are popped");
		return std::move(inputs);
	}
	++_physicsSimulationsCounter;
	return std::nullopt;
}
