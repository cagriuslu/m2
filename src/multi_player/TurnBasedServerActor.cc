#include <m2/multi_player/TurnBasedServerActor.h>
#include <m2/Game.h>

namespace {
	constexpr uint16_t TCP_PORT_NO = 1162;
	constexpr auto CLIENT_RECONNECT_TIMEOUT_MS = 30000;
}

bool m2::TurnBasedServerActor::Initialize(MessageBox<TurnBasedServerActorInput>&, MessageBox<TurnBasedServerActorOutput>& outbox) {
	CreateSocket();
	BindSocket();
	StartListening(outbox);
	StartPingBroadcast();
	return true;
}

bool m2::TurnBasedServerActor::operator()(MessageBox<TurnBasedServerActorInput>& inbox, MessageBox<TurnBasedServerActorOutput>& outbox) {
	ProcessInbox(inbox, outbox);
	if (_state == pb::SERVER_GAME_FINISHED) {
		return false;
	}
	ProcessReceivedMessages(outbox);
	CheckDisconnectedClients(outbox);
	const auto preSelectHandles = GetSocketHandlesToReadAndWrite();
	const auto postSelectHandles = SelectSockets(preSelectHandles);
	if (not postSelectHandles) {
		return true; // Timeout occurred, try again later
	}
	const auto& [readableSockets, writeableSockets] = *postSelectHandles;
	CheckConnectionListeningSocket(outbox, readableSockets);
	CheckReadableClientSockets(readableSockets, outbox);
	CheckWritableClientSockets(writeableSockets);

	return true;
}

void m2::TurnBasedServerActor::CreateSocket() {
	_connectionListeningSocket = network::TcpSocket::CreateServerSideSocket(TCP_PORT_NO);
	if (not _connectionListeningSocket) {
		throw M2_ERROR("TcpSocket creation failed: " + _connectionListeningSocket.error());
	}
	LOG_DEBUG("TcpSocket created");
}
void m2::TurnBasedServerActor::BindSocket() {
	// The previous socket may linger up to 30 seconds
	m2Repeat(32) {
		if (auto bindResult = _connectionListeningSocket->bind(); not bindResult) {
			throw M2_ERROR("Bind failed: " + bindResult.error());
		} else {
			if (bindResult.value()) {
				LOG_DEBUG("TcpSocket bound");
				return;
			}
			LOG_INFO("TcpSocket is busy, waiting 1s before retrying binding");
			std::this_thread::sleep_for(std::chrono::seconds{1});
		}
	}
	throw M2_ERROR("Bind failed: Address already in use");
}
void m2::TurnBasedServerActor::StartListening(MessageBox<TurnBasedServerActorOutput>& outbox) {
	if (auto listenResult = _connectionListeningSocket->listen(I(_maxConnCount)); not listenResult) {
		throw M2_ERROR("Listen failed: " + listenResult.error());
	}
	LOG_INFO("TcpSocket listening on port", TCP_PORT_NO);
	SetStateAndPublish(outbox, pb::SERVER_LOBBY_OPEN);
}
void m2::TurnBasedServerActor::StartPingBroadcast() {
	// Only implemented for macOS
#ifndef _WIN32
	// Start ping broadcast
	_pingBroadcastThread.emplace();
#endif
}

void m2::TurnBasedServerActor::ProcessInbox(MessageBox<TurnBasedServerActorInput>& inbox, MessageBox<TurnBasedServerActorOutput>& outbox) {
	// Process only one message
	if (std::optional<TurnBasedServerActorInput> msg; inbox.PopMessage(msg) && msg) {
		if (std::holds_alternative<TurnBasedServerActorInput::CloseLobby>(msg->variant)) {
			if (_state != pb::SERVER_LOBBY_OPEN) {
				throw M2_ERROR("Received unexpected lobby closure command");
			}
			if (not std::ranges::all_of(_clients, network::is_client_ready)) {
				LOG_WARN("Unable to close lobby, not every client is ready");
			} else {
				_pingBroadcastThread.reset();
				SetStateAndPublish(outbox, pb::SERVER_LOBBY_CLOSED);
			}
		} else if (std::holds_alternative<TurnBasedServerActorInput::UpdateTurnHolder>(msg->variant)) {
			_turnHolderIndex = std::get<TurnBasedServerActorInput::UpdateTurnHolder>(msg->variant).clientIndex;
		} else if (std::holds_alternative<TurnBasedServerActorInput::SendServerUpdate>(msg->variant)) {
			// Start the game if not already done so
			if (_state == pb::SERVER_LOBBY_CLOSED) {
				SetStateAndPublish(outbox, pb::SERVER_STARTED);
			} else if (_state == pb::SERVER_STARTED) {
				// Do nothing
			} else {
				throw M2_ERROR("Received unexpected server update command");
			}
			// Send updates
			auto serverUpdateCopy = std::get<TurnBasedServerActorInput::SendServerUpdate>(msg->variant).serverUpdate;
			for (auto i = 1; i < I(_clients.size()); ++i) { // TurnBasedServerUpdate is not sent to self
				if (_clients[i].is_ready()) {
					LOG_DEBUG("Queueing TurnBasedServerUpdate to client", i);
					serverUpdateCopy.mutable_server_update()->set_receiver_index(i);
					_clients[i].queue_outgoing_message(serverUpdateCopy);
				}
			}
			// Shutdown if necessary
			if (serverUpdateCopy.server_update().shutdown()) {
				for (auto& client : _clients) {
					client.flush_and_shutdown();
				}
				SetStateAndPublish(outbox, pb::SERVER_GAME_FINISHED);
				return;
			} else {
				_lastServerUpdate = std::move(serverUpdateCopy); // Save for later
			}
		} else if (std::holds_alternative<TurnBasedServerActorInput::SendServerCommand>(msg->variant)) {
			if (_state != pb::SERVER_LOBBY_CLOSED && _state != pb::SERVER_STARTED) {
				throw M2_ERROR("Received unexpected TurnBasedServerCommand command");
			}
			const auto& serverCommand = std::get<TurnBasedServerActorInput::SendServerCommand>(msg->variant).serverCommand;
			const auto queueMsg = [this, &serverCommand](const int receiverIndex) {
				pb::TurnBasedNetworkMessage message;
				message.set_game_hash(M2_GAME.Hash());
				message.mutable_server_command()->CopyFrom(serverCommand);
				if (_clients[receiverIndex].is_ready()) {
					message.set_sequence_no(_clients[receiverIndex].ReturnAndIncrementServerCommandSequenceNo());
					LOG_DEBUG("Queueing TurnBasedServerCommand to client", receiverIndex);
					_clients[receiverIndex].queue_outgoing_message(std::move(message));
				} else {
					LOG_WARN("Attempted to queue TurnBasedServerCommand but client is disconnected");
				}
			};
			if (const auto receiverIndex = std::get<TurnBasedServerActorInput::SendServerCommand>(msg->variant).receiverIndex; receiverIndex < 0) {
				for (int i = 0; i < I(_clients.size()); ++i) {
					queueMsg(i);
				}
			} else {
				queueMsg(receiverIndex);
			}
		} else {
			throw M2_ERROR("Unexpected message type");
		}
	}
}
void m2::TurnBasedServerActor::ProcessReceivedMessages(MessageBox<TurnBasedServerActorOutput>& outbox) {
	// Process up to one message from each client
	for (auto i = 0; i < I(_clients.size()); ++i) {
		auto& client = _clients[i];
		if (not client.has_incoming_data(false)) {
			continue;
		}
		// Peek cannot be null if has_incoming_data is true
		if (const auto* peek = client.peek_incoming_message(); peek && peek->has_client_update()) {
			// Check sequence no
			if (peek->sequence_no() < client.expectedClientUpdateSequenceNo) {
				LOG_WARN("Ignoring TurnBasedClientUpdate with an outdated sequence number", peek->sequence_no());
				client.pop_incoming_message(); // Message "handled"
			} else if (client.expectedClientUpdateSequenceNo < peek->sequence_no()) {
				LOG_WARN("TurnBasedClientUpdate with an unexpected sequence number received, kicking client", peek->sequence_no());
				client.set_misbehaved();
				HandleMisbehavedClient(outbox, i);
			} else {
				++client.expectedClientUpdateSequenceNo;
				if (_state == pb::SERVER_LOBBY_OPEN) {
					LOG_INFO("Received client ready token", i, peek->client_update().ready_token());
					client.set_ready_token(peek->client_update().ready_token());
					client.pop_incoming_message(); // Message handled
					PublishStateUpdate(outbox); // Ready client count changed
				} else if (_state == pb::SERVER_STARTED && client.is_untrusted()) {
					if (client.set_ready_token(peek->client_update().ready_token())) {
						if (_lastServerUpdate) {
							LOG_INFO("Previously reconnected client has presented the correct ready token, will send TurnBasedServerUpdate", i);
							_lastServerUpdate->mutable_server_update()->set_receiver_index(i);
							_clients[i].queue_outgoing_message(*_lastServerUpdate);
						}
					} else {
						LOG_INFO("Previously reconnected client presented incorrect ready token, disconnecting client", i);
						client.honorably_disconnect();
						HandleDisconnectedClient(outbox, i);
					}
					client.pop_incoming_message(); // Message handled
				} else {
					LOG_WARN("Received unexpected TurnBasedClientUpdate", i);
					client.set_misbehaved();
					HandleMisbehavedClient(outbox, i);
				}
			}
		} else if (peek && peek->has_client_command()) {
			// Check sequence number
			if (peek->sequence_no() < client.expectedClientCommandSequenceNo) {
				LOG_WARN("Ignoring TurnBasedClientCommand with an outdated sequence number", peek->sequence_no());
				client.pop_incoming_message(); // Message "handled"
			} else if (client.expectedClientCommandSequenceNo < peek->sequence_no()) {
				LOG_WARN("TurnBasedClientCommand with an unexpected sequence number received, closing client", peek->sequence_no());
				client.set_misbehaved();
				HandleMisbehavedClient(outbox, i);
			} else {
				client.expectedClientCommandSequenceNo++;
				if (_turnHolderIndex != i) {
					LOG_WARN("Received TurnBasedClientCommand from a non-turn-holder client", i);
					client.set_misbehaved();
					HandleMisbehavedClient(outbox, i);
				} else {
					LOG_INFO("TurnBasedClientCommand from player index with sequence number received, will be processed by game loop", i, peek->sequence_no());
					outbox.PushMessage(TurnBasedServerActorOutput{
						.variant = TurnBasedServerActorOutput::ClientEvent{
							.eventVariant = TurnBasedServerActorOutput::ClientEvent::CommandFromTurnHolder{
								.turnHolderCommand = std::move(*client.pop_incoming_message()) // Message handled
							}
						}
					});
				}
			}
		} else {
			LOG_WARN("Received unexpected message from client", i);
			client.set_misbehaved();
			HandleMisbehavedClient(outbox, i);
		}
	}
}
void m2::TurnBasedServerActor::CheckDisconnectedClients(MessageBox<TurnBasedServerActorOutput>& outbox) {
	for (auto i = 0; i < I(_clients.size()); ++i) {
		if (auto disconnected_since = _clients[i].disconnected_or_untrusted_since();
			disconnected_since && *disconnected_since + CLIENT_RECONNECT_TIMEOUT_MS < sdl::get_ticks()) {
			HandleDisconnectedClient(outbox, i);
		}
	}
}
std::optional<m2::TurnBasedServerActor::ReadAndWriteTcpSocketHandles> m2::TurnBasedServerActor::SelectSockets(const ReadAndWriteTcpSocketHandles& preSelectHandles) {
	auto selectResult = network::Select{}(preSelectHandles.first, preSelectHandles.second, 250);
	if (not selectResult) {
		throw M2_ERROR("Select failed: " + selectResult.error());
	}
	return *selectResult;
}
void m2::TurnBasedServerActor::CheckConnectionListeningSocket(MessageBox<TurnBasedServerActorOutput>& outbox, const network::TcpSocketHandles& readableSockets) {
	// Check if main socket is readable
	if (not std::ranges::contains(readableSockets, &*_connectionListeningSocket)) {
		return;
	}
	LOG_INFO("Main socket is readable");

	if (auto clientSocket = _connectionListeningSocket->accept(); not clientSocket) {
		throw M2_ERROR("Accept failed: " + clientSocket.error());
	} else if (not clientSocket->has_value()) {
		LOG_WARN("Client aborted connection by the time it was accepted");
	} else {
		if (_maxConnCount <= I(_clients.size())) {
			LOG_INFO("Refusing connection because of connection limit", (*clientSocket)->GetClientIpAddressAndPort());
		} else if (_state == pb::SERVER_LOBBY_OPEN) {
			LOG_INFO("New client connected with index", _clients.size(), (*clientSocket)->GetClientIpAddressAndPort());
			_clients.emplace_back(std::move(**clientSocket), I(_clients.size()));
			PublishStateUpdate(outbox);
		} else if (_state == pb::SERVER_LOBBY_CLOSED) {
			LOG_INFO("Refusing connection to closed lobby", (*clientSocket)->GetClientIpAddressAndPort());
		} else if (_state == pb::SERVER_STARTED) {
			// Check if there's a disconnected client
			bool found = false;
			for (int i = 0; i < I(_clients.size()) && not found; ++i) {
				if (auto& client = _clients[i]; client.is_disconnected()
						&& client.ip_address_and_port() == (*clientSocket)->GetClientIpAddressAndPort()) {
					LOG_INFO("Previously connected client with index connected again", i, (*clientSocket)->GetClientIpAddressAndPort());
					client.untrusted_client_reconnected(std::move(**clientSocket));
					found = true;
				}
			}
			if (not found) {
				LOG_INFO("Refusing connection after game started", (*clientSocket)->GetClientIpAddressAndPort());
			}
		} else {
			throw M2_ERROR("Unexpected state");
		}
	}
}
void m2::TurnBasedServerActor::CheckReadableClientSockets(const network::TcpSocketHandles& readableSockets, MessageBox<TurnBasedServerActorOutput>& outbox) {
	for (auto& client : _clients) {
		if (not client.is_connected()) {
			// Skip client if it's connection has dropped
			continue;
		}
		// Read from socket
		const auto isReadable = std::ranges::find(readableSockets, &client.tcp_socket()) != readableSockets.end();
		client.has_incoming_data(isReadable);
	}
	// If the lobby is not yet closed, remove disconnected clients
	if (_state == pb::ServerThreadState::SERVER_LOBBY_OPEN) {
		if (const auto eraseIt = std::ranges::remove_if(_clients, &network::TurnBasedClientManager::is_disconnected_or_untrusted).begin();
				eraseIt != _clients.end()) {
			_clients.erase(eraseIt, _clients.end());
			PublishStateUpdate(outbox);
		}
	}
}
void m2::TurnBasedServerActor::CheckWritableClientSockets(const network::TcpSocketHandles& writeableSockets) {
	for (auto& client : _clients) {
		if (not client.is_ready() || std::ranges::find(writeableSockets, &client.tcp_socket()) == writeableSockets.end()) {
			// Skip if the connection has dropped or the socket is not writeable
			continue;
		}
		// Write to socket
		client.send_outgoing_data();
	}
}

m2::TurnBasedServerActor::ReadAndWriteTcpSocketHandles m2::TurnBasedServerActor::GetSocketHandlesToReadAndWrite() {
	network::TcpSocketHandles socketsToRead;
	socketsToRead.emplace_back(&*_connectionListeningSocket); // Add the main socket
	for (auto& client : _clients) {
		if (client.is_connected()) {
			// Add connected sockets as readable
			socketsToRead.emplace_back(&client.tcp_socket());
		}
	}

	network::TcpSocketHandles socketsToWrite;
	for (auto& client : _clients) {
		if (client.is_ready() && client.has_outgoing_data()) {
			// Add ready clients which have outgoing data as writeable
			socketsToWrite.emplace_back(&client.tcp_socket());
		}
	}

	return {socketsToRead, socketsToWrite};
}
void m2::TurnBasedServerActor::SetStateAndPublish(MessageBox<TurnBasedServerActorOutput>& outbox, const pb::ServerThreadState newState) {
	_state = newState;
	PublishStateUpdate(outbox);
}
void m2::TurnBasedServerActor::PublishStateUpdate(MessageBox<TurnBasedServerActorOutput>& outbox) const {
	outbox.PushMessage(TurnBasedServerActorOutput{.variant = TurnBasedServerActorOutput::StateUpdate{
		.threadState = _state,
		.clientCount = I(_clients.size()),
		.readyClientCount = I(std::ranges::count_if(_clients, network::is_client_ready))
	}});
}
void m2::TurnBasedServerActor::HandleDisconnectedClient(MessageBox<TurnBasedServerActorOutput>& outbox, const int clientIndex) {
	// TODO handle with a pre-determined strategy
	outbox.PushMessage(TurnBasedServerActorOutput{.variant = TurnBasedServerActorOutput::ClientEvent{.eventVariant = TurnBasedServerActorOutput::ClientEvent::DisconnectedClient{.clientIndex = clientIndex}}});
}
void m2::TurnBasedServerActor::HandleMisbehavedClient(MessageBox<TurnBasedServerActorOutput>& outbox, const int clientIndex) {
	// TODO handle with a pre-determined strategy
	outbox.PushMessage(TurnBasedServerActorOutput{.variant = TurnBasedServerActorOutput::ClientEvent{.eventVariant = TurnBasedServerActorOutput::ClientEvent::DisconnectedClient{.clientIndex = clientIndex}}});
}
