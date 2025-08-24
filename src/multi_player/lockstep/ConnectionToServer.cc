#include <m2/multi_player/lockstep/ConnectionToServer.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

namespace {
	constexpr int N_RESPONSES_TO_ASSUME_CONNECTION = 3;
}

ConnectionToServer::ConnectionToServer(network::IpAddressAndPort serverAddress, MessagePasser& messagePasser, MessageBox<ClientActorOutput>& clientOutbox)
	: _serverAddressAndPort(std::move(serverAddress)), _messagePasser(messagePasser),
	_state([&clientOutbox](const State& newState) {
		clientOutbox.PushMessage(ClientActorOutput{
			.variant = ClientActorOutput::ConnectionToServerStateUpdate{
				.stateIndex = newState.index()
			},
			.gameInitParams = std::holds_alternative<LobbyFrozen>(newState)
				? std::optional{std::get<LobbyFrozen>(newState).gameInitParams}
				: std::optional<m2g::pb::LockstepGameInitParams>{}
		});
	}, State{}) {}

void ConnectionToServer::SetReadyState(const bool readyState) {
	if (std::holds_alternative<WaitingInLobby>(_state.Get())) {
		if (std::get<WaitingInLobby>(_state.Get()).readyState != readyState) {
			pb::LockstepMessage msg;
			msg.set_set_ready_state(readyState);
			LOG_INFO("Queueing readiness message", readyState);
			QueueOutgoingMessage(std::move(msg));
			_state.Mutate([&](State& state) {
				std::get<WaitingInLobby>(state).readyState = readyState;
			});
		}
	} else {
		throw M2_ERROR("Attempt to set ready state outside of the lobby");
	}
}
void ConnectionToServer::QueueOutgoingMessages() {
	const ConnectionStatistics* connStats = _messagePasser.GetConnectionStatistics(_serverAddressAndPort);
	if (std::holds_alternative<SearchForServer>(_state.Get())) {
		if (not connStats) {
			LOG_DEBUG("Queueing first ping toward server");
			QueueOutgoingMessage({});
		} else if (const auto nAckedMsgs = connStats->GetTotalAckedOutgoingSmallMessages(); nAckedMsgs < N_RESPONSES_TO_ASSUME_CONNECTION) {
			// Check if previous ping have been ACKed
			if (connStats->GetTotalQueuedOutgoingSmallMessages() == nAckedMsgs) {
				LOG_DEBUG("Queueing another ping toward server");
				QueueOutgoingMessage({});
			}
		} else { // nAckedMsgs == N_RESPONSES_TO_ASSUME_CONNECTION
			// Enough pings have been made
			_state.Emplace(WaitingInLobby{});
		}
	} else if (std::holds_alternative<WaitingInLobby>(_state.Get())) {
		// TODO
	}
}
void ConnectionToServer::DeliverIncomingMessage(pb::LockstepMessage&& msg) {
	if (msg.has_set_ready_state()) {
		LOG_WARN("Server sent readiness message");
	} else if (msg.has_freeze_lobby_with_init_params()) {
		if (std::holds_alternative<WaitingInLobby>(_state.Get())) {
			LOG_INFO("Received lobby freeze message, closing lobby");
			_state.Emplace(LobbyFrozen{
				.gameInitParams = msg.freeze_lobby_with_init_params()
			});
		} else {
			LOG_WARN("Received lobby freeze message when the lobby isn't open");
		}
	}
}

void ConnectionToServer::QueueOutgoingMessage(pb::LockstepMessage&& msg) {
	_messagePasser.QueueMessage(MessageAndReceiver{
		.message = std::move(msg),
		.receiver = _serverAddressAndPort
	});
}
