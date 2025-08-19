#include <m2/multi_player/lockstep/ConnectionToServer.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

namespace {
	constexpr int N_RESPONSES_TO_ASSUME_CONNECTION = 3;
}

ConnectionToServer::ConnectionToServer(MessageBox<ClientActorOutput>& clientOutbox) : _state([&clientOutbox](const State& newState) {
	clientOutbox.PushMessage(ClientActorOutput{
		.variant = ClientActorOutput::ConnectionToServerStateUpdate{
			.stateIndex = newState.index()
		}
	});
}, State{}) {}

void ConnectionToServer::GatherOutgoingMessages(const ConnectionStatistics* connStats, std::queue<pb::LockstepMessage>& out) {
	if (std::holds_alternative<SearchForServer>(_state.Get())) {
		if (not connStats) {
			LOG_DEBUG("Queueing first ping toward server");
			out.emplace();
		} else if (const auto nAckedMsgs = connStats->GetTotalAckedOutgoingPackets(); nAckedMsgs < N_RESPONSES_TO_ASSUME_CONNECTION) {
			// Check if previous ping have been ACKed
			if (connStats->GetTotalQueuedOutgoingPackets() == nAckedMsgs) {
				LOG_DEBUG("Queueing another ping toward server");
				out.emplace();
			}
		} else { // nAckedMsgs == N_RESPONSES_TO_ASSUME_CONNECTION
			// Enough pings have been made
			_state.Set(WaitForPlayers{});
		}
	} else if (std::holds_alternative<WaitForPlayers>(_state.Get())) {
		// TODO
	}
}

void ConnectionToServer::DeliverIncomingMessage(const ConnectionStatistics*, pb::LockstepMessage&& in) {
	// TODO
}
