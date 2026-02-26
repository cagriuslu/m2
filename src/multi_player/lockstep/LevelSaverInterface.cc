#include <m2/multi_player/lockstep/LevelSaverInterface.h>
#include <filesystem>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

void LevelSaverInterface::StorePlayerInputs(const network::Timecode timecode, std::vector<std::deque<m2g::pb::LockstepPlayerInput>>&& playerInputs) {
	GetActorInbox().PushMessage(LevelSaverInput{
		.timecode = timecode,
		.playerInputs = std::move(playerInputs)
	});
}
