#include <m2/multi_player/lockstep/ConnectionToServer.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

namespace {
	constexpr unsigned N_RESPONSES_TO_ASSUME_CONNECTION = 3;
	constexpr std::chrono::milliseconds CONNECTION_ATTEMPT_PERIOD{500};
}

std::optional<ControlMessage> ConnectionToServer::GatherOutgoingMessages(std::queue<pb::LockstepMessage>& out) {
	if (std::holds_alternative<SearchingForServer>(_state)) {
		if (auto& searchingForServer = std::get<SearchingForServer>(_state);
				searchingForServer.responsesReceived < N_RESPONSES_TO_ASSUME_CONNECTION) {
			if (not searchingForServer.lastConnectionAttemptAt
				|| CONNECTION_ATTEMPT_PERIOD < searchingForServer.lastConnectionAttemptAt->GetDurationSinceMs()) {
				LOG_INFO("Queueing ping in search of the server");
				out.emplace(); // Empty message is a ping
				searchingForServer.lastConnectionAttemptAt = Stopwatch{};
				return std::nullopt;
			} else {
				return std::nullopt; // Wait before sending another ping
			}
		} else {
			throw M2_ERROR("Client received enough responses from the server, but the state is not advanced");
		}
	}
	// TODO
	return std::nullopt;
}

std::optional<ControlMessage> ConnectionToServer::DeliverIncomingMessage(pb::LockstepMessage&& in) {
	// TODO
	return std::nullopt;
}
