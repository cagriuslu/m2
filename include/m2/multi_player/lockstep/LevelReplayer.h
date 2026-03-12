#pragma once
#include <m2/network/Types.h>
#include <M2.orm.h>
#include <genORM/genORM.h>

namespace m2::multiplayer::lockstep {
	class LevelReplayer {
		genORM::database _db;
		int _playerCount;
		int32_t _physicsSimulationsCounter{};
		network::Timecode _nextTimecode{};

	public:
		explicit LevelReplayer(genORM::database&& db, const int playerCount) : _db(std::move(db)), _playerCount(playerCount) {}

		[[nodiscard]] bool IsFinished() const;

		struct SimulationInputs {
			network::Timecode timecode;
			std::vector<std::deque<m2g::pb::LockstepPlayerInput>> allInputs;
		};
		std::optional<SimulationInputs> GetNextSimulationInputs();
	};
}
