#include <m2/multi_player/lockstep/LevelSaver.h>
#include <m2/Log.h>
#include <M2.orm.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

LevelSaver::LevelSaver(std::unique_ptr<genORM::database> db) : ActorBase(), _db(std::move(db)) {}

const char* LevelSaver::ThreadNameForLogging() const { return "LS"; }

bool LevelSaver::operator()(MessageBox<LevelSaverInput>& inbox, MessageBox<LevelSaverOutput>&) {
	if (inbox.WaitMessage([](const LevelSaverInput&) { return true; },
			std::chrono::duration_cast<Stopwatch::Duration>(std::chrono::milliseconds{100}))) {
		if (std::optional<LevelSaverInput> msg; inbox.TryPopMessage(msg) && msg) {
			LOG_NETWORK("Received player inputs to save");
			for (int playerIndex = 0; playerIndex < I(msg->playerInputs.size()); ++playerIndex) {
				for (const auto& playerInput : msg->playerInputs[playerIndex]) {
					LOG_NETWORK("Persisting player input for timecode to database", msg->timecode, playerIndex);
					const auto serialized = playerInput.SerializeAsString();
					std::vector<uint8_t> bytes{serialized.begin(), serialized.end()};
					if (const auto createResult = orm::LocktepPlayerInput::create(*_db, I(msg->timecode), playerIndex, std::move(bytes));
							not createResult) {
						LOG_ERROR("Unable to persist LockstepPlayerInput to database", createResult.error());
						return false;
					}
				}
			}
		}
	}
	return true;
}
