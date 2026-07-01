#include <m2/multiplayer/TurnBasedClientActor.h>
#include <m2/network/TcpSocket.h>
#include <m2/network/Select.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/multiplayer/turnbased/MessagePasser.h>

bool m2::network::detail::TurnBasedClientActor::Initialize(MessageBox<std::monostate>&, MessageBox<std::monostate>&) {
	LOG_INFO("TurnBasedClientActor function");
	return true;
}

bool m2::network::detail::TurnBasedClientActor::operator()(MessageBox<std::monostate>&, MessageBox<std::monostate>&) {
	auto& sharedState = *_sharedState;

	auto locked_get_client_state = [&sharedState] {
		const std::lock_guard lock(sharedState.mutex);
		return sharedState.state;
	};
	auto unlocked_set_state = [&sharedState](const pb::ClientThreadState newState) {
		LOG_NETWORK("Setting ClientThread state", pb::enum_name(newState));
		sharedState.state = newState;
	};
	auto locked_set_state = [&sharedState, &unlocked_set_state](const pb::ClientThreadState newState) {
		const std::lock_guard lock(sharedState.mutex);
		unlocked_set_state(newState);
	};
	auto locked_has_outgoing_message = [&sharedState] {
		const std::lock_guard lock(sharedState.mutex);
		return not sharedState.outgoingQueue.empty();
	};
	auto locked_has_incoming_message = [&sharedState] {
		const std::lock_guard lock(sharedState.mutex);
		return not sharedState.incomingQueue.empty();
	};
	auto locked_has_unprocessed_server_update_or_command = [&sharedState] {
		const std::lock_guard lock(sharedState.mutex);
		return static_cast<bool>(sharedState.receivedServerUpdate) || static_cast<bool>(sharedState.receivedServerCommand);
	};

	// Replaces the loop's continuation condition. When the state machine reaches a terminal state, stop the actor.
	if (const auto terminalState = locked_get_client_state();
			terminalState == pb::CLIENT_SHUTDOWN
			|| terminalState == pb::CLIENT_QUIT
			|| terminalState == pb::CLIENT_RECONNECTION_TIMEOUT_QUIT) {
		return false;
	}

	if (auto state = locked_get_client_state();
		state == pb::CLIENT_INITIAL_STATE || state == pb::CLIENT_RECONNECTING) {
		// Sanity check
		if (std::holds_alternative<multiplayer::turnbased::MessagePasser>(_socketManagerOrTicksDisconnectedAt)) {
			throw M2_ERROR("Unexpected socket");
		}

		// Check if we should give up trying to reconnect
		if (state == pb::CLIENT_RECONNECTING) {
			if (not std::holds_alternative<thirdparty::video::Ticks>(_socketManagerOrTicksDisconnectedAt)) {
				throw M2_ERROR("Expected ticks disconnected at");
			}
			if (std::get<thirdparty::video::Ticks>(_socketManagerOrTicksDisconnectedAt) + 30000 < thirdparty::video::GetTicks()) {
				LOG_WARN("Time out while trying to reconnect to server");
				unlocked_set_state(pb::CLIENT_RECONNECTION_TIMEOUT_QUIT);
				return true;
			}
		}

		// Non-blocking back-off between connection attempts (was a blocking sleep_for(1s) after a timed-out connect).
		if (_retryConnectionAfterTicks) {
			if (thirdparty::video::GetTicks() < *_retryConnectionAfterTicks) {
				return true; // Still backing off; retry on a later pass
			}
			_retryConnectionAfterTicks.reset();
		}

		// Socket not yet created, or we just got disconnected. Create socket.
		auto socket = TcpSocket::CreateClientSideSocket(_serverAddress, options::GetPort());
		if (not socket) {
			LOG_FATAL("TcpSocket creation failed", socket.error());
			return false;
		}

		// Attempt to connect
		if (auto connect_success = socket->connect(); not connect_success) {
			throw M2_ERROR("Connect failed: " + connect_success.error());
		} else {
			if (not *connect_success) {
				LOG_INFO("Connection timed out, will retry after 1s");
				_retryConnectionAfterTicks = thirdparty::video::GetTicks() + 1000;
				return true;
				// We cannot reuse the socket for retrying
			} else {
				LOG_INFO("Established connection to server");
				if (state == pb::CLIENT_INITIAL_STATE) {
					// If the server has reached the maximum number of players, it'll accept connections and
					// immediately close them. We need to check if the socket is still connected.
					// Prepare socket handles for Select
					TcpSocketHandles sockets_to_read;
					sockets_to_read.emplace_back(&*socket);
					// Select
					LOG_INFO("Waiting 100ms in case the server rejects the new connection");
					auto select_result = Select::WaitUntilSocketsReady(sockets_to_read, {}, 100);
					if (not select_result) {
						throw M2_ERROR("Select failed: " + select_result.error());
					}
					if (*select_result == std::nullopt) {
						// Timeout occurred, all good, the socket isn't closed.
						_socketManagerOrTicksDisconnectedAt.emplace<multiplayer::turnbased::MessagePasser>(std::move(*socket), -1);
						locked_set_state(pb::CLIENT_CONNECTED);
					} else if (not select_result.value().value().readableSockets.empty()) {
						// Server should not have sent anything until we signalled as ready.
						LOG_WARN("Connection was closed from server because the socket is readable immediately upon connection");
						// This means (most likely) that the server has disconnected the socket.
						locked_set_state(pb::CLIENT_QUIT);
					} else {
						throw M2_ERROR("Unexpected select result");
					}
				} else if (state == pb::CLIENT_RECONNECTING) {
					_socketManagerOrTicksDisconnectedAt.emplace<multiplayer::turnbased::MessagePasser>(std::move(*socket), -1);
					locked_set_state(pb::CLIENT_RECONNECTED);
				} else {
					throw M2_ERROR("Unexpected state");
				}
			}
		}
	} else {
		if (not std::holds_alternative<multiplayer::turnbased::MessagePasser>(_socketManagerOrTicksDisconnectedAt)) {
			throw M2_ERROR("Expected socket");
		}
		auto& socket_manager = std::get<multiplayer::turnbased::MessagePasser>(_socketManagerOrTicksDisconnectedAt);
		// Connected, reconnected, ready, or started

		// Wait until the previous TurnBasedServerUpdate & TurnBasedServerCommand is processed (was a blocking
		// while-sleep_for loop; now a non-blocking early return that resumes on a later pass).
		if (locked_has_unprocessed_server_update_or_command()) {
			return true;
		}

		// Preprocess only one incoming message to not block the other incoming/outgoing messages
		if (locked_has_incoming_message()) {
			const std::lock_guard lock(sharedState.mutex);

			// Pop the front message
			auto front_message = std::move(sharedState.incomingQueue.front());
			sharedState.incomingQueue.pop();
			if (front_message.has_server_update()) {
				if (sharedState.state == pb::CLIENT_CONNECTED || sharedState.state == pb::CLIENT_RECONNECTED) {
					LOG_WARN("TurnBasedServerUpdate received from server while not ready, closing client");
					unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
					return true;
				} else if (not sharedState.receivedServerUpdate) {
					// Ensure that the state is STARTED
					if (sharedState.state == pb::CLIENT_READY) {
						unlocked_set_state(pb::CLIENT_STARTED);
					}
					auto received_level_token = front_message.server_update().level_token();
					if (_levelToken == 0) {
						// Record level token
						_levelToken = received_level_token;
					} else if (_levelToken == received_level_token) {
						// All good
					} else {
						LOG_WARN("Mismatching level token after reconnection, closing client");
						unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
						return true;
					}
					// Check sequence number
					if (front_message.sequence_no() < _expectedServerUpdateSequenceNo) {
						LOG_WARN("Ignoring TurnBasedServerUpdate with an outdated sequence number", front_message.sequence_no());
					} else if (_expectedServerUpdateSequenceNo < front_message.sequence_no()) {
						LOG_WARN("TurnBasedServerUpdate with an unexpected sequence number received, closing client", front_message.sequence_no());
						unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
						return true;
					} else {
						LOG_INFO("TurnBasedServerUpdate with sequence number received", front_message.sequence_no());
						_expectedServerUpdateSequenceNo++;
						auto* server_update = front_message.release_server_update();
						sharedState.receivedServerUpdate.emplace(std::make_pair(front_message.sequence_no(), std::move(*server_update)));
						delete server_update;
					}
				}
			} else if (front_message.has_server_command()) {
				if (sharedState.state != pb::CLIENT_STARTED) {
					LOG_WARN("TurnBasedServerCommand received from server while not ready, closing client");
					unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
					return true;
				}
				// Check sequence number
				if (front_message.sequence_no() < _expectedServerCommandSequenceNo) {
					LOG_WARN("Ignoring TurnBasedServerCommand with an outdated sequence number", front_message.sequence_no());
				} else if (_expectedServerCommandSequenceNo < front_message.sequence_no()) {
					LOG_WARN("TurnBasedServerCommand with an unexpected sequence number received, closing client", front_message.sequence_no());
					unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
					return true;
				} else {
					LOG_INFO("TurnBasedServerCommand with sequence number received", front_message.sequence_no());
					_expectedServerCommandSequenceNo++;
					auto* server_command = front_message.release_server_command();
					sharedState.receivedServerCommand.emplace(std::make_pair(front_message.sequence_no(), std::move(*server_command)));
					delete server_command;
				}
			} else {
				LOG_WARN("Unsupported message received from server, closing client");
				unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
				return true;
			}
		}

		// Prepare socket handles for Select
		TcpSocketHandles sockets_to_read, sockets_to_write;
		sockets_to_read.emplace_back(&socket_manager.socket());
		if (socket_manager.has_outgoing_data() || locked_has_outgoing_message()) {
			sockets_to_write.emplace_back(&socket_manager.socket());
		}
		// Select
		auto select_result = Select::WaitUntilSocketsReady(sockets_to_read, sockets_to_write, 250);
		if (not select_result) {
			throw M2_ERROR("Select failed: " + select_result.error());
		}
		if (not *select_result) {
			// Timeout occurred
			return true;
		}
		// If there's anything to read
		if (not select_result.value().value().readableSockets.empty()) {
			const std::lock_guard lock(sharedState.mutex);
			auto read_result = socket_manager.read_incoming_data(sharedState.incomingQueue);
			if (not read_result) {
				LOG_WARN("Error occurred while reading, closing connection to server", read_result.error());
				_socketManagerOrTicksDisconnectedAt.emplace<thirdparty::video::Ticks>(thirdparty::video::GetTicks());
				unlocked_set_state(pb::CLIENT_RECONNECTING);
				return true;
			} else if (*read_result != multiplayer::turnbased::MessagePasser::ReadResult::MESSAGE_RECEIVED && *read_result != multiplayer::turnbased::MessagePasser::ReadResult::INCOMPLETE_MESSAGE_RECEIVED) {
				LOG_WARN("Invalid data received from server, closing client", static_cast<int>(*read_result));
				unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
				return true;
			}
		}
		// If there's anything to write
		if (not select_result.value().value().writableSockets.empty()) {
			const std::lock_guard lock(sharedState.mutex);
			auto send_result = socket_manager.send_outgoing_data(sharedState.outgoingQueue);
			if (not send_result) {
				LOG_WARN("Error occurred while writing, closing connection to server", send_result.error());
				_socketManagerOrTicksDisconnectedAt.emplace<thirdparty::video::Ticks>(thirdparty::video::GetTicks());
				unlocked_set_state(pb::CLIENT_RECONNECTING);
				return true;
			} else if (*send_result != multiplayer::turnbased::MessagePasser::SendResult::OK) {
				throw M2_ERROR(std::format("An invalid or too large outgoing message was queued to server: {}", static_cast<int>(*send_result)));
			}
		}
	}
	return true;
}
