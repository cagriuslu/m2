#include <../../include/m2/multi_player/TurnBasedClientThreadBase.h>
#include <m2/network/TcpSocket.h>
#include <m2/network/Select.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/network/TcpSocketManager.h>

m2::network::detail::TurnBasedClientThreadBase::TurnBasedClientThreadBase(mplayer::Type type, std::string addr, bool ping_broadcast)
		: _type(type), _addr(std::move(addr)), _ping_broadcast(ping_broadcast), _ready_token(RandomNonZero64()),
		_thread(base_client_thread_func, this) {
	LOG_INFO("Constructing ClientThread with ready-token", _ready_token);
}

m2::network::detail::TurnBasedClientThreadBase::~TurnBasedClientThreadBase() {
	DEBUG_FN();
	locked_set_state(pb::CLIENT_QUIT);
	if (_thread.joinable()) {
		// If the object is default created, thread may not be joinable
		_thread.join();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(250)); // Give some time for the client thread to initiate TCP shutdown
}

m2::pb::ClientThreadState m2::network::detail::TurnBasedClientThreadBase::locked_get_client_state() {
	const std::lock_guard lock(_mutex);
	return _state;
}
bool m2::network::detail::TurnBasedClientThreadBase::locked_has_server_update() {
	const std::lock_guard lock(_mutex);
	return static_cast<bool>(_received_server_update);
}
const m2::pb::TurnBasedServerUpdate* m2::network::detail::TurnBasedClientThreadBase::locked_peek_server_update() {
	const std::lock_guard lock(_mutex);
	if (_received_server_update) {
		return &_received_server_update->second;
	}
	return nullptr;
}
std::optional<std::pair<m2::SequenceNo,m2::pb::TurnBasedServerUpdate>> m2::network::detail::TurnBasedClientThreadBase::locked_pop_server_update() {
	const std::lock_guard lock(_mutex);
	if (_received_server_update) {
		auto tmp = std::move(_received_server_update);
		_received_server_update.reset();
		return tmp;
	} else {
		return std::nullopt;
	}
}
bool m2::network::detail::TurnBasedClientThreadBase::locked_has_server_command() {
	const std::lock_guard lock(_mutex);
	return static_cast<bool>(_received_server_command);
}
std::optional<std::pair<m2::SequenceNo,m2g::pb::TurnBasedServerCommand>> m2::network::detail::TurnBasedClientThreadBase::locked_pop_server_command() {
	const std::lock_guard lock(_mutex);
	if (_received_server_command) {
		auto tmp = std::move(_received_server_command);
		_received_server_command.reset();
		return tmp;
	} else {
		return std::nullopt;
	}
}

void m2::network::detail::TurnBasedClientThreadBase::locked_set_ready(bool ready) {
	auto state = locked_get_client_state();
	if (ready) {
		if (state != pb::CLIENT_CONNECTED && state != pb::CLIENT_RECONNECTED) {
			throw M2_ERROR("Unexpected state while signaling readiness");
		}
	} else {
		if (state != pb::CLIENT_READY) {
			throw M2_ERROR("Unexpected state while signaling unreadiness");
		}
	}

	LOG_INFO("Will send ready state", ready);

	{
		const std::lock_guard lock(_mutex);
		pb::TurnBasedNetworkMessage msg;
		msg.set_game_hash(M2_GAME.Hash());
		msg.mutable_client_update()->set_ready_token(ready ? _ready_token : 0);
		_outgoing_queue.push(std::move(msg));
		LOG_DEBUG("Readiness message queued");
	}

	auto locked_has_outgoing_message = [this]() {
		const std::lock_guard lock(_mutex);
		return not _outgoing_queue.empty();
	};

	do {
		// Wait until output queue is empty
		LOG_DEBUG("Waiting 100ms until outgoing message queue is empty");
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} while (locked_has_outgoing_message());
	locked_set_state(state ? pb::CLIENT_READY : pb::CLIENT_CONNECTED);
}
void m2::network::detail::TurnBasedClientThreadBase::locked_start_if_ready() {
	if (locked_get_client_state() == pb::CLIENT_READY) {
		locked_set_state(pb::CLIENT_STARTED);
	}
}
void m2::network::detail::TurnBasedClientThreadBase::locked_queue_client_command(const m2g::pb::TurnBasedClientCommand& cmd) {
	INFO_FN();

	pb::TurnBasedNetworkMessage msg;
	msg.set_game_hash(M2_GAME.Hash());
	msg.set_sequence_no(_nextClientCommandSequenceNo++);
	msg.mutable_client_command()->CopyFrom(cmd);

	{
		const std::lock_guard lock(_mutex);
		_outgoing_queue.push(std::move(msg));
	}
}
void m2::network::detail::TurnBasedClientThreadBase::locked_shutdown() {
	locked_set_state(pb::CLIENT_SHUTDOWN);
}

void m2::network::detail::TurnBasedClientThreadBase::unlocked_set_state(pb::ClientThreadState state) {
	LOG_DEBUG("Setting ClientThread state", pb::enum_name(state));
	_state = state;
}

void m2::network::detail::TurnBasedClientThreadBase::locked_set_state(pb::ClientThreadState state) {
	const std::lock_guard lock(_mutex);
	unlocked_set_state(state);
}

void m2::network::detail::TurnBasedClientThreadBase::base_client_thread_func(TurnBasedClientThreadBase* thread_manager) {
	thread_manager->_latch.wait();
	SetThreadNameForLogging(thread_manager->thread_name());
	LOG_INFO("TurnBasedClientThreadBase function");

	auto locked_should_continue_running = [](TurnBasedClientThreadBase* thread_manager) {
		auto current_state = thread_manager->locked_get_client_state();
		return current_state != pb::CLIENT_SHUTDOWN
			&& current_state != pb::CLIENT_QUIT
			&& current_state != pb::CLIENT_RECONNECTION_TIMEOUT_QUIT;
	};
	auto locked_has_outgoing_message = [](TurnBasedClientThreadBase* thread_manager) {
		const std::lock_guard lock(thread_manager->_mutex);
		return not thread_manager->_outgoing_queue.empty();
	};
	auto locked_has_incoming_message = [](TurnBasedClientThreadBase* thread_manager) {
		const std::lock_guard lock(thread_manager->_mutex);
		return not thread_manager->_incoming_queue.empty();
	};
	auto locked_has_unprocessed_server_update_or_command = [](TurnBasedClientThreadBase* thread_manager) {
		const std::lock_guard lock(thread_manager->_mutex);
		return thread_manager->_received_server_update || thread_manager->_received_server_command;
	};

	std::variant<std::monostate, TcpSocketManager, sdl::ticks_t> socket_manager_or_ticks_disconnected_at;
	std::optional<PingBroadcastThread> ping_broadcast_thread;
	while (locked_should_continue_running(thread_manager)) {
		if (auto state = thread_manager->locked_get_client_state();
			state == pb::CLIENT_INITIAL_STATE || state == pb::CLIENT_RECONNECTING) {
			// Sanity check
			if (std::holds_alternative<TcpSocketManager>(socket_manager_or_ticks_disconnected_at)) {
				throw M2_ERROR("Unexpected socket");
			}

			// Check if we should give up trying to reconnect
			if (state == pb::CLIENT_RECONNECTING) {
				if (not std::holds_alternative<sdl::ticks_t>(socket_manager_or_ticks_disconnected_at)) {
					throw M2_ERROR("Expected ticks disconnected at");
				}
				if (std::get<sdl::ticks_t>(socket_manager_or_ticks_disconnected_at) + 30000 < sdl::get_ticks()) {
					LOG_WARN("Time out while trying to reconnect to server");
					thread_manager->unlocked_set_state(pb::CLIENT_RECONNECTION_TIMEOUT_QUIT);
					continue;
				}
			}

			// Socket not yet created, or we just got disconnected. Create socket.
			auto socket = TcpSocket::create_client(thread_manager->_addr, 1162);
			if (not socket) {
				LOG_FATAL("TcpSocket creation failed", socket.error());
				return;
			}
            // Ping broadcast for Windows is not implemented
#ifndef _WIN32
			// Start ping broadcast if enabled
			if (thread_manager->_ping_broadcast && not ping_broadcast_thread) {
				ping_broadcast_thread.emplace();
				// Wait some time before attempting to connect
				LOG_DEBUG("Waiting 1s for ping broadcasts");
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
#endif

			// Attempt to connect
			if (auto connect_success = socket->connect(); not connect_success) {
				throw M2_ERROR("Connect failed: " + connect_success.error());
			} else {
				if (not *connect_success) {
					LOG_INFO("Connection timed out, waiting 1s before retrying");
					std::this_thread::sleep_for(std::chrono::seconds(1));
					// We cannot reuse the socket for retrying
				} else {
					LOG_INFO("Established connection to server");
					ping_broadcast_thread.reset(); // Stop ping broadcast
					if (state == pb::CLIENT_INITIAL_STATE) {
						// If the server has reached the maximum number of players, it'll accept connections and
						// immediately close them. We need to check if the socket is still connected.
						// Prepare socket handles for Select
						TcpSocketHandles sockets_to_read;
						sockets_to_read.emplace_back(&*socket);
						// Select
						LOG_INFO("Waiting 100ms in case the server rejects the new connection");
						auto select_result = Select{}(sockets_to_read, {}, 100);
						if (not select_result) {
							throw M2_ERROR("Select failed: " + select_result.error());
						}
						if (*select_result == std::nullopt) {
							// Timeout occurred, all good, the socket isn't closed.
							socket_manager_or_ticks_disconnected_at.emplace<TcpSocketManager>(std::move(*socket), -1);
							thread_manager->locked_set_state(pb::CLIENT_CONNECTED);
						} else if (not select_result.value().value().first.empty()) {
							// Server should not have sent anything until we signalled as ready.
							LOG_WARN("Connection was closed from server because the socket is readable immediately upon connection");
							// This means (most likely) that the server has disconnected the socket.
							thread_manager->locked_set_state(pb::CLIENT_QUIT);
						} else {
							throw M2_ERROR("Unexpected select result");
						}
					} else if (state == pb::CLIENT_RECONNECTING) {
						socket_manager_or_ticks_disconnected_at.emplace<TcpSocketManager>(std::move(*socket), -1);
						thread_manager->locked_set_state(pb::CLIENT_RECONNECTED);
					} else {
						throw M2_ERROR("Unexpected state");
					}
				}
			}
		} else {
			if (not std::holds_alternative<TcpSocketManager>(socket_manager_or_ticks_disconnected_at)) {
				throw M2_ERROR("Expected socket");
			}
			auto& socket_manager = std::get<TcpSocketManager>(socket_manager_or_ticks_disconnected_at);
			// Connected, reconnected, ready, or started

			// Wait until the previous TurnBasedServerUpdate & TurnBasedServerCommand is processed
			while (locked_has_unprocessed_server_update_or_command(thread_manager)) {
				LOG_DEBUG("Waiting 250ms until unprocessed server updates or commands to be processed");
				std::this_thread::sleep_for(std::chrono::milliseconds(250));
			}

			// Preprocess only one incoming message to not block the other incoming/outgoing messages
			if (locked_has_incoming_message(thread_manager)) {
				const std::lock_guard lock(thread_manager->_mutex);

				// Pop the front message
				auto front_message = std::move(thread_manager->_incoming_queue.front());
				thread_manager->_incoming_queue.pop();
				if (front_message.has_server_update()) {
					if (thread_manager->_state == pb::CLIENT_CONNECTED || thread_manager->_state == pb::CLIENT_RECONNECTED) {
						LOG_WARN("TurnBasedServerUpdate received from server while not ready, closing client");
						thread_manager->unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
						continue;
					} else if (not thread_manager->_received_server_update) {
						// Ensure that the state is STARTED
						if (thread_manager->_state == pb::CLIENT_READY) {
							thread_manager->unlocked_set_state(pb::CLIENT_STARTED);
						}
						auto received_level_token = front_message.server_update().level_token();
						if (thread_manager->_level_token == 0) {
							// Record level token
							thread_manager->_level_token = received_level_token;
						} else if (thread_manager->_level_token == received_level_token) {
							// All good
						} else {
							LOG_WARN("Mismatching level token after reconnection, closing client");
							thread_manager->unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
							continue;
						}
						// Check sequence number
						if (front_message.sequence_no() < thread_manager->_expectedServerUpdateSequenceNo) {
							LOG_WARN("Ignoring TurnBasedServerUpdate with an outdated sequence number", front_message.sequence_no());
						} else if (thread_manager->_expectedServerUpdateSequenceNo < front_message.sequence_no()) {
							LOG_WARN("TurnBasedServerUpdate with an unexpected sequence number received, closing client", front_message.sequence_no());
							thread_manager->unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
							continue;
						} else {
							LOG_INFO("TurnBasedServerUpdate with sequence number received", front_message.sequence_no());
							thread_manager->_expectedServerUpdateSequenceNo++;
							auto* server_update = front_message.release_server_update();
							thread_manager->_received_server_update.emplace(std::make_pair(front_message.sequence_no(), std::move(*server_update)));
							delete server_update;
						}
					}
				} else if (front_message.has_server_command()) {
					if (thread_manager->_state != pb::CLIENT_STARTED) {
						LOG_WARN("TurnBasedServerCommand received from server while not ready, closing client");
						thread_manager->unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
						continue;
					}
					// Check sequence number
					if (front_message.sequence_no() < thread_manager->_expectedServerCommandSequenceNo) {
						LOG_WARN("Ignoring TurnBasedServerCommand with an outdated sequence number", front_message.sequence_no());
					} else if (thread_manager->_expectedServerCommandSequenceNo < front_message.sequence_no()) {
						LOG_WARN("TurnBasedServerCommand with an unexpected sequence number received, closing client", front_message.sequence_no());
						thread_manager->unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
						continue;
					} else {
						LOG_INFO("TurnBasedServerCommand with sequence number received", front_message.sequence_no());
						thread_manager->_expectedServerCommandSequenceNo++;
						auto* server_command = front_message.release_server_command();
						thread_manager->_received_server_command.emplace(std::make_pair(front_message.sequence_no(), std::move(*server_command)));
						delete server_command;
					}
				} else {
					LOG_WARN("Unsupported message received from server, closing client");
					thread_manager->unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
					continue;
				}
			}

			// Prepare socket handles for Select
			TcpSocketHandles sockets_to_read, sockets_to_write;
			sockets_to_read.emplace_back(&socket_manager.socket());
			if (socket_manager.has_outgoing_data() || locked_has_outgoing_message(thread_manager)) {
				sockets_to_write.emplace_back(&socket_manager.socket());
			}
			// Select
			auto select_result = Select{}(sockets_to_read, sockets_to_write, 250);
			if (not select_result) {
				throw M2_ERROR("Select failed: " + select_result.error());
			}
			if (not *select_result) {
				// Timeout occurred
				continue;
			}
			// If there's anything to read
			if (not select_result.value().value().first.empty()) {
				const std::lock_guard lock(thread_manager->_mutex);
				auto read_result = socket_manager.read_incoming_data(thread_manager->_incoming_queue);
				if (not read_result) {
					LOG_WARN("Error occurred while reading, closing connection to server", read_result.error());
					socket_manager_or_ticks_disconnected_at.emplace<sdl::ticks_t>(sdl::get_ticks());
					thread_manager->unlocked_set_state(pb::CLIENT_RECONNECTING);
					continue;
				} else if (*read_result != TcpSocketManager::ReadResult::MESSAGE_RECEIVED && *read_result != TcpSocketManager::ReadResult::INCOMPLETE_MESSAGE_RECEIVED) {
					LOG_WARN("Invalid data received from server, closing client", static_cast<int>(*read_result));
					thread_manager->unlocked_set_state(pb::CLIENT_MISBEHAVING_SERVER_QUIT);
					continue;
				}
			}
			// If there's anything to write
			if (not select_result.value().value().second.empty()) {
				const std::lock_guard lock(thread_manager->_mutex);
				auto send_result = socket_manager.send_outgoing_data(thread_manager->_outgoing_queue);
				if (not send_result) {
					LOG_WARN("Error occurred while writing, closing connection to server", send_result.error());
					socket_manager_or_ticks_disconnected_at.emplace<sdl::ticks_t>(sdl::get_ticks());
					thread_manager->unlocked_set_state(pb::CLIENT_RECONNECTING);
					continue;
				} else if (*send_result != TcpSocketManager::SendResult::OK) {
					throw M2_ERROR("An invalid or too large outgoing message was queued to server: " + m2::ToString(static_cast<int>(*send_result)));
				}
			}
		}
	}
}
