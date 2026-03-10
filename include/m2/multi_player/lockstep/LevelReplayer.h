#pragma once
#include <m2/network/Types.h>
#include <M2.orm.h>
#include <genORM/genORM.h>

namespace m2::multiplayer::lockstep {
	class LevelReplayer {
		genORM::database _db;
		int32_t _physicsSimulationsCounter{};

	public:
		explicit LevelReplayer(genORM::database&& db) : _db(std::move(db)) {}

		[[nodiscard]] bool IsFinished() const;

		struct SimulationInputs {
			network::Timecode timecode;
			std::vector<std::deque<m2g::pb::LockstepPlayerInput>> allInputs;
		};
		std::optional<SimulationInputs> GetNextSimulationInputs();
	};
}
