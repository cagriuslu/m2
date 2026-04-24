#include <m2/multiplayer/lockstep/LevelSaver.h>
#include <m2/Log.h>
#include <m2/thirdparty/compression/Deflate.h>
#include <M2.orm.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

LevelSaver::LevelSaver(std::unique_ptr<genORM::database> db) : ActorBase(), _db(std::move(db)) {}

const char* LevelSaver::ThreadNameForLogging() const { return "LS"; }

bool LevelSaver::operator()(MessageBox<LevelSaverInput>& inbox, MessageBox<LevelSaverOutput>&) {
	if (inbox.WaitMessage([](const LevelSaverInput&) { return true; }, std::chrono::duration_cast<Stopwatch::Duration>(std::chrono::milliseconds{100}))) {
		if (std::optional<LevelSaverInput> msg; inbox.TryPopMessage(msg) && msg) {
			if (std::holds_alternative<LevelSaverInput::PlayerInputs>(msg->variant)) {
				LOG_NETWORK("Received player inputs to save");
				const auto timecode = std::get<LevelSaverInput::PlayerInputs>(msg->variant).timecode;
				const auto& playerInputs = std::get<LevelSaverInput::PlayerInputs>(msg->variant).inputs;
				for (int playerIndex = 0; playerIndex < I(playerInputs.size()); ++playerIndex) {
					for (const auto& playerInput : playerInputs[playerIndex]) {
						LOG_NETWORK("Persisting player input for timecode to database", timecode, playerIndex);
						const auto serialized = playerInput.SerializeAsString();
						std::vector<uint8_t> bytes{serialized.begin(), serialized.end()};
						if (const auto createResult = orm::LocktepPlayerInput::create(*_db, I(timecode), playerIndex, std::move(bytes)); not createResult) {
							LOG_ERROR("Unable to persist LockstepPlayerInput to database", createResult.error());
							return false;
						}
					}
				}
			} else if (std::holds_alternative<LevelSaverInput::DebugStateReport>(msg->variant)) {
				const auto timecode = std::get<LevelSaverInput::DebugStateReport>(msg->variant).timecode;
				const auto& report = std::get<LevelSaverInput::DebugStateReport>(msg->variant).report;
				LOG_NETWORK("Persisting debug state report for timecode with hash to database", timecode, report.game_state_hash());
				const auto serialized = report.SerializeAsString();
				const auto uncompressed = std::vector<uint8_t>{serialized.begin(), serialized.end()};
				auto compressed = thirdparty::compression::Deflate(uncompressed);
				m2SucceedOrThrowError(compressed);
				if (const auto createResult = orm::LockstepDebugStateReport::create(*_db, I(timecode), std::move(*compressed)); not createResult) {
					LOG_ERROR("Unable to persist LockstepDebugStateReport to database", createResult.error());
					return false;
				}
			} else {
				throw M2_ERROR("Unknown input variant");
			}
		}
	}
	return true;
}
