#pragma once
#include "LevelSaverInputOutput.h"
#include <m2/mt/actor/ActorBase.h>
#include <genORM/genORM.h>

namespace m2::multiplayer::lockstep {
	class LevelSaver final : public ActorBase<LevelSaverInput,LevelSaverOutput> {
		std::unique_ptr<genORM::database> _db;

	public:
		explicit LevelSaver(std::unique_ptr<genORM::database> db);

		[[nodiscard]] const char* ThreadNameForLogging() const override;

		bool Initialize(MessageBox<LevelSaverInput>&, MessageBox<LevelSaverOutput>&) override { return true; }

		bool operator()(MessageBox<LevelSaverInput>&, MessageBox<LevelSaverOutput>&) override;

		void Deinitialize(MessageBox<LevelSaverInput>&, MessageBox<LevelSaverOutput>&) override {}
	};
}