#pragma once
#include "LevelSaver.h"
#include <m2/mt/actor/ActorInterfaceBase.h>
#include <genORM/genORM.h>

namespace m2::multiplayer::lockstep {
	class LevelSaverInterface final : public ActorInterfaceBase<LevelSaver> {
	public:
		explicit LevelSaverInterface(std::unique_ptr<genORM::database> db) : ActorInterfaceBase(std::move(db)) {}

		void StorePlayerInputs(network::Timecode, std::vector<std::pair<std::deque<m2g::pb::LockstepPlayerInput>, uint64_t>> playerInputs);
		void StoreDebugStateReport(network::Timecode, pb::LockstepDebugStateReport&& report);
	};
}
