#include <m2/multi_player/lockstep/LevelSaverInterface.h>
#include <m2/Log.h>
#include <filesystem>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

void LevelSaverInterface::StorePlayerInputs(const network::Timecode timecode, std::vector<std::deque<m2g::pb::LockstepPlayerInput>>&& playerInputs) {
	LOG_NETWORK("Sending player inputs to level saver");
	GetActorInbox().PushMessage(LevelSaverInput{
		.timecode = timecode,
		.playerInputs = std::move(playerInputs)
	});
}
