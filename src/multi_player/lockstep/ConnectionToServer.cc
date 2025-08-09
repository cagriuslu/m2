#include <m2/multi_player/lockstep/ConnectionToServer.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

namespace {
	constexpr int N_RESPONSES_TO_ASSUME_CONNECTION = 3;
}

void ConnectionToServer::GatherOutgoingMessages(ConnectionStatistics* connStats, std::queue<pb::LockstepMessage>& out) {
	if (std::holds_alternative<SearchingForServer>(_state)) {
		if (not connStats) {
			out.emplace(); // Send the first ping
		} else if (const auto nAckedMsgs = connStats->GetTotalAckedOutgoingPackets(); nAckedMsgs < N_RESPONSES_TO_ASSUME_CONNECTION) {
			if (connStats->GetTotalQueuedOutgoingPackets() == nAckedMsgs) {
				// All pings have been ACKed, send another ping
				out.emplace();
			}
		} else {
			// TODO check connection quality
		}
	}
	// TODO
}

void ConnectionToServer::DeliverIncomingMessage(ConnectionStatistics*, pb::LockstepMessage&& in) {
	// TODO
}
