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
	if (++_physicsSimulationsCounter == m2g::LOCKSTEP_PHYSICS_SIMULATION_COUNT_PER_GAME_TICK) {
		const auto timecodeToFetch = _nextTimecode++;

		LOG_NETWORK("Fetching players inputs from save file for timecode...", timecodeToFetch);
		auto expectPlayerInputs = orm::LocktepPlayerInput::find_all_by_timecode(_db, timecodeToFetch);
		m2SucceedOrThrowError(expectPlayerInputs);

		SimulationInputs inputs;
		inputs.timecode = timecodeToFetch;
		inputs.allInputs.resize(_playerCount);
		int nonEmptyInputCount = 0;
		for (const auto& fetchedInput : *expectPlayerInputs) {
			const auto playerIndex = fetchedInput.get_player_index();
			if (_playerCount <= playerIndex) {
				throw M2_ERROR("Save file contains a player with an invalid index");
			}
			m2g::pb::LockstepPlayerInput input;
			{
				if (const auto& inputBytes = fetchedInput.get_player_input(); not inputBytes.empty()) {
					if (not input.ParseFromArray(inputBytes.data(), inputBytes.size())) {
						throw M2_ERROR("Unable to parse a player input from the save file");
					}
				}
			}
			inputs.allInputs[playerIndex].emplace_back(std::move(input));
			++nonEmptyInputCount;
		}
		LOG_NETWORK("Fetched players inputs from save file for timecode", timecodeToFetch, nonEmptyInputCount);

		_physicsSimulationsCounter = 0;
		return std::move(inputs);
	}
	return std::nullopt;
}
