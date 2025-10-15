#include <m2/multi_player/lockstep/ConnectionToServer.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

namespace {
	constexpr int N_RESPONSES_TO_ASSUME_CONNECTION = 3;

	/// A concept that requires a state to have a peerList of type PeerList
	template <typename StateT>
	concept StateWithPeerList = std::same_as<decltype(std::declval<StateT>().peerList), ConnectionToServer::PeerList>;
}

void ConnectionToServer::PeerList::Update(const pb::LockstepPeerDetails& details, MessagePasser& messagePasser) {
	_peers.resize(details.peers_size());
	for (int i = 0; i < details.peers_size(); ++i) {
		if (i == details.receiver_index()) {
			if (_peers[i]) {
				throw M2_ERROR("Replacing peers is not yet supported");
			}
			LOG_INFO("Self index is received", i);
			_peers[i] = std::nullopt; // Self
		} else {
			const auto& newPeer = details.peers(i);
			const auto newPeerIpPort = network::IpAddressAndPort{
				.ipAddress = network::IpAddress::CreateFromNetworkOrder(newPeer.ip()),
				.port = network::Port::CreateFromNetworkOrder(newPeer.port())
			};
			if (_peers[i]) {
				if (_peers[i]->GetAddressAndPort() != newPeerIpPort) {
					throw M2_ERROR("Replacing peers is not yet supported");
				}
			} else {
				LOG_INFO("Adding a new peer with index", newPeerIpPort, i);
				_peers[i].emplace(ConnectionToPeer{newPeerIpPort, messagePasser});
			}
		}
	}
	_connectionStateRequiresReporting = true;
}
void ConnectionToServer::PeerList::ReportIfAllPeersConnected(MessagePasser& messagePasser, const network::IpAddressAndPort& serverAddressAndPort) {
	if (_connectionStateRequiresReporting) {
		bool isAllConnected = true;
		for (const auto& peer : _peers) {
			if (peer && peer->IsConnected() == false) {
				isAllConnected = false;
				break;
			}
		}
		if (isAllConnected) {
			pb::LockstepMessage msg;
			msg.set_all_peers_reachable(true);
			LOG_INFO("Reporting all peers as reachable");
			messagePasser.QueueMessage(MessageAndReceiver{
				.message = std::move(msg),
				.receiver = serverAddressAndPort
			});
			_connectionStateRequiresReporting = false;
		}
	}
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
void ConnectionToServer::MarkGameAsStarted() {
	if (std::holds_alternative<GameStarted>(_state.Get())) {
		// Already started
	} else if (std::holds_alternative<LobbyFrozen>(_state.Get())) {
		LOG_INFO("Marking game as started...");
		_state.Mutate([](auto& state) {
			state.template emplace<GameStarted>(GameStarted{
				.peerList = std::move(std::get<LobbyFrozen>(state).peerList)
			});
		});
	} else {
		throw M2_ERROR("Attempt to mark game as started outside of frozen lobby state");
	}
}
void ConnectionToServer::QueueOutgoingMessages(const std::optional<network::Timecode> timecode, const std::deque<m2g::pb::LockstepPlayerInput>* unsentPlayerInputs) {
	const auto queuePlayerInputs = [this](const network::Timecode timecode_, const std::deque<m2g::pb::LockstepPlayerInput>& playerInputs) {
		pb::LockstepMessage msg;
		msg.mutable_player_inputs()->set_timecode(timecode_); // Set this option even if there are no inputs
		for (const auto& playerInput : playerInputs) {
			msg.mutable_player_inputs()->add_player_inputs()->CopyFrom(playerInput);
		}
		QueueOutgoingMessage(std::move(msg));
	};

	if (std::holds_alternative<SearchForServer>(_state.Get())) {
		if (const auto* connStats = _messagePasser.GetConnectionStatistics(_serverAddressAndPort); not connStats) {
			LOG_DEBUG("Queueing first ping toward server");
			QueueOutgoingMessage({});
		} else if (const auto nAckedMsgs = connStats->GetTotalAckedOutgoingSmallMessages(); nAckedMsgs < N_RESPONSES_TO_ASSUME_CONNECTION) {
			// Not enough ping-pongs have been made with the server. Ping the server if all previous pings have been ACKed.
			if (connStats->GetTotalQueuedOutgoingSmallMessages() == nAckedMsgs) {
				LOG_DEBUG("Queueing another ping toward server");
				QueueOutgoingMessage({});
			}
		} else { // nAckedMsgs == N_RESPONSES_TO_ASSUME_CONNECTION
			// Enough ping-pongs have been made with the server. Assume that we're placed in the server's lobby.
			_state.Mutate([&](auto& state) {
				state.template emplace<WaitingInLobby>(WaitingInLobby{
					.peerList = std::move(std::get<SearchForServer>(state).peerList)
				});
			});
		}
	} else if (std::holds_alternative<WaitingInLobby>(_state.Get())) {
		_state.MutateNoSideEffect([this](auto& state) {
			auto& peerList = std::get<WaitingInLobby>(state).peerList;
			peerList.ReportIfAllPeersConnected(_messagePasser, _serverAddressAndPort);
			for (auto& peer : peerList) {
				if (peer) { peer->QueueOutgoingMessages(); }
			}
		});
	} else if (std::holds_alternative<LobbyFrozen>(_state.Get()) && timecode && unsentPlayerInputs) {
		queuePlayerInputs(*timecode, *unsentPlayerInputs);
	} else if (std::holds_alternative<GameStarted>(_state.Get()) && timecode && unsentPlayerInputs) {
		queuePlayerInputs(*timecode, *unsentPlayerInputs);
	}
}
void ConnectionToServer::DeliverIncomingMessage(pb::LockstepMessage&& msg) {
	if (msg.has_peer_details()) {
		_state.Mutate([this, &msg](auto& state) {
			std::visit(overloaded{
				[this, &msg](StateWithPeerList auto& s) { s.peerList.Update(msg.peer_details(), _messagePasser); },
				[](const auto&) { throw M2_ERROR("Received peer details during an invalid state"); }
			}, state);
		});
	} else if (msg.has_set_ready_state()) {
		LOG_WARN("Server sent unexpected message: Ready state");
	} else if (msg.has_freeze_lobby_with_init_params()) {
		if (std::holds_alternative<WaitingInLobby>(_state.Get())) {
			LOG_INFO("Received lobby freeze message, closing lobby");
			_state.Mutate([&](auto& state) {
				state.template emplace<LobbyFrozen>(LobbyFrozen{
					.gameInitParams = msg.freeze_lobby_with_init_params(),
					.peerList = std::move(std::get<WaitingInLobby>(state).peerList)
				});
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
