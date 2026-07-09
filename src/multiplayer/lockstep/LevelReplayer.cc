#include <m2/multiplayer/lockstep/LevelReplayer.h>
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
	if (++_physicsSimulationsCounter == m2g::LOCKSTEP_PHYSICS_SIMULATION_COUNT_PER_GAME_TICK) {
		const auto timecodeToFetch = _nextTimecode++;

		LOG_NETWORK("Fetching players inputs from save file for timecode...", timecodeToFetch);
		const auto expectPlayerInputs = orm::LockstepPlayerInput::find_all_by_timecode(_db, timecodeToFetch);
		m2SucceedOrThrowError(expectPlayerInputs);

		SimulationInputs inputs;
		inputs.timecode = timecodeToFetch;
		inputs.allInputs.resize(_playerCount);
		for (const auto& fetchedInput : *expectPlayerInputs) {
			const auto playerIndex = fetchedInput.get_player_index();
			if (_playerCount <= playerIndex) {
				throw M2_ERROR("Save file contains a player with an invalid index");
			}
			pb::LockstepPlayerInputs envelope;
			{
				if (const auto& envelopeBytes = fetchedInput.get_player_input(); not envelopeBytes.empty()) {
					if (not envelope.ParseFromArray(envelopeBytes.data(), envelopeBytes.size())) {
						throw M2_ERROR("Unable to parse LockstepPlayerInputs from the save file");
					}
				}
			}
			inputs.allInputs[playerIndex].first.assign(envelope.player_inputs().begin(), envelope.player_inputs().end());
			inputs.allInputs[playerIndex].second = envelope.rng_seed();
		}
		LOG_NETWORK("Fetched players inputs from save file for timecode", timecodeToFetch);

		_physicsSimulationsCounter = 0;
		return std::move(inputs);
	}
	return std::nullopt;
}
