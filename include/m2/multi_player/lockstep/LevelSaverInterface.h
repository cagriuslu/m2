#pragma once
#include "LevelSaver.h"
#include <m2/mt/actor/ActorInterfaceBase.h>
#include <genORM/genORM.h>

namespace m2::multiplayer::lockstep {
	class LevelSaverInterface final : public ActorInterfaceBase<LevelSaver, LevelSaverInput, LevelSaverOutput> {
	public:
		explicit LevelSaverInterface(std::unique_ptr<genORM::database> db) : ActorInterfaceBase(std::move(db)) {}

		void StorePlayerInputs(network::Timecode, std::vector<std::deque<m2g::pb::LockstepPlayerInput>>&& playerInputs);
	};
}
