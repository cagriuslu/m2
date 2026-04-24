#include <m2/multiplayer/lockstep/LevelSaverInterface.h>
#include <m2/Log.h>
#include <filesystem>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

void LevelSaverInterface::StorePlayerInputs(const network::Timecode timecode, std::vector<std::deque<m2g::pb::LockstepPlayerInput>> playerInputs) {
	LOG_NETWORK("Sending player inputs to level saver");
	GetActorInbox().PushMessage(LevelSaverInput{
		.variant = LevelSaverInput::PlayerInputs{
			.timecode = timecode,
			.inputs = std::move(playerInputs)
		}
	});
}
void LevelSaverInterface::StoreDebugStateReport(const network::Timecode timecode, pb::LockstepDebugStateReport&& report) {
	LOG_NETWORK("Sending debug state report to level saver");
	GetActorInbox().PushMessage(LevelSaverInput{
		.variant = LevelSaverInput::DebugStateReport{
			.timecode = timecode,
			.report = std::move(report)
		}
	});
}
