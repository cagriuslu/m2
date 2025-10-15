#include <m2/multi_player/lockstep/ConnectionToPeer.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

namespace {
	constexpr int N_RESPONSES_TO_ASSUME_CONNECTION = 3;
}

void ConnectionToPeer::QueueOutgoingMessages() {
	const auto queuePing = [this]() {
		_messagePasser.QueueMessage(MessageAndReceiver{
			.message = {},
			.receiver = _addressAndPort
		});
	};

	if (std::holds_alternative<SearchForPeer>(_state)) {
		if (const auto* connStats = _messagePasser.GetConnectionStatistics(_addressAndPort); not connStats) {
			LOG_DEBUG("Queueing first ping toward peer", _addressAndPort);
			queuePing();
		} else if (const auto nAckedMsgs = connStats->GetTotalAckedOutgoingSmallMessages(); nAckedMsgs < N_RESPONSES_TO_ASSUME_CONNECTION) {
			// Not enough ping-pongs have been made with the peer. Ping the peer if all previous pings have been ACKed.
			if (connStats->GetTotalQueuedOutgoingSmallMessages() == nAckedMsgs) {
				LOG_DEBUG("Queueing another ping toward peer", _addressAndPort);
				queuePing();
			}
		} else { // nAckedMsgs == N_RESPONSES_TO_ASSUME_CONNECTION
			// Enough ping-pongs have been made with the peer.
			LOG_INFO("Connected to peer", _addressAndPort);
			_state = ConnectedToPeer{};
		}
	}
}
