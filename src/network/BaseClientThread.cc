#include <m2/network/BaseClientThread.h>
#include <m2/network/Socket.h>
#include <m2/network/Detail.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/Meta.h>
#include <unistd.h>

m2::network::detail::BaseClientThread::BaseClientThread(mplayer::Type type, std::string addr, bool ping_broadcast)
	: _type(type), _addr(std::move(addr)), _ping_broadcast(ping_broadcast), _thread(BaseClientThread::base_client_thread_func, this) {
	INFO_FN();
}

m2::network::detail::BaseClientThread::~BaseClientThread() {
	DEBUG_FN();
	locked_set_state(pb::CLIENT_QUIT);
	if (_thread.joinable()) {
		// If the object is default created, thread may not be joinable
		_thread.join();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(250)); // Give some time for the client thread to initiate TCP shutdown
}

m2::pb::ClientState m2::network::detail::BaseClientThread::locked_get_client_state() {
	const std::lock_guard lock(_mutex);
	return _state;
}

bool m2::network::detail::BaseClientThread::locked_has_server_update() {
	const std::lock_guard lock(_mutex);
	return static_cast<bool>(_received_server_update);
}

const m2::pb::ServerUpdate* m2::network::detail::BaseClientThread::locked_peek_server_update() {
	const std::lock_guard lock(_mutex);
	if (_received_server_update) {
		return &*_received_server_update;
	} else {
		return nullptr;
	}
}

std::optional<m2::pb::ServerUpdate> m2::network::detail::BaseClientThread::locked_pop_server_update() {
	const std::lock_guard lock(_mutex);
	if (_received_server_update) {
		auto tmp = std::move(_received_server_update);
		_received_server_update = {};
		return tmp;
	} else {
		return std::nullopt;
	}
}

bool m2::network::detail::BaseClientThread::locked_has_server_command() {
	const std::lock_guard lock(_mutex);
	return static_cast<bool>(_received_server_command);
}

std::optional<m2g::pb::ServerCommand> m2::network::detail::BaseClientThread::locked_pop_server_command() {
	const std::lock_guard lock(_mutex);
	if (_received_server_command) {
		auto tmp = std::move(_received_server_command);
		_received_server_command = {};
		return tmp;
	} else {
		return std::nullopt;
	}
}

void m2::network::detail::BaseClientThread::locked_set_ready_sync(bool state) {
	LOG_INFO("Setting ready state", state);

	{
		const std::lock_guard lock(_mutex);
		pb::NetworkMessage msg;
		msg.set_game_hash(M2_GAME.hash());
		msg.set_ready(state);
		LOG_DEBUG("Readiness message queued");
		_out_message_queue.emplace_back(std::move(msg));
	}

	while (locked_get_outgoing_message_count()) {
		SDL_Delay(100); // Wait until output queue is empty
	}
	LOG_DEBUG("Output queue flushed");

	locked_set_state(state ? pb::CLIENT_READY : pb::CLIENT_CONNECTED);
}

void m2::network::detail::BaseClientThread::locked_queue_client_command(const m2g::pb::ClientCommand& cmd) {
	INFO_FN();

	pb::NetworkMessage msg;
	msg.set_game_hash(M2_GAME.hash());
	msg.mutable_client_command()->CopyFrom(cmd);

	const std::lock_guard lock(_mutex);
	_out_message_queue.emplace_back(std::move(msg));
}

bool m2::network::detail::BaseClientThread::locked_should_continue_running() {
	auto current_state = locked_get_client_state();
	return current_state != pb::CLIENT_QUIT && current_state != pb::CLIENT_SHUTDOWN;
}

size_t m2::network::detail::BaseClientThread::locked_get_outgoing_message_count() {
	const std::lock_guard lock(_mutex);
	return _out_message_queue.size();
}

void m2::network::detail::BaseClientThread::unlocked_set_state(pb::ClientState state) {
	LOG_DEBUG("Setting state", pb::enum_name(state));
	_state = state;
}

void m2::network::detail::BaseClientThread::locked_set_state(pb::ClientState state) {
	const std::lock_guard lock(_mutex);
	unlocked_set_state(state);
}

m2::pb::NetworkMessage m2::network::detail::BaseClientThread::locked_unsafe_pop_outgoing_message() {
	const std::lock_guard lock(_mutex);
	auto tmp = std::move(_out_message_queue.front()); // unsafe front() access
	_out_message_queue.pop_front();
	return tmp;
}

void m2::network::detail::BaseClientThread::base_client_thread_func(BaseClientThread* thread_manager) {
	set_thread_name_for_logging("CL");
	LOG_INFO("BaseClientThread function");

	std::optional<Socket> socket;
	while (thread_manager->locked_should_continue_running()) {
		if (auto state = thread_manager->locked_get_client_state(); state == pb::CLIENT_NOT_READY) {
			// If not yet connected

			// Create socket
			auto expect_socket = Socket::create();
			if (not expect_socket) {
				LOG_FATAL("Socket creation failed", expect_socket.error());
				return;
			}
			socket = std::move(*expect_socket);
			LOG_INFO("Socket created");

			// Start ping broadcast if enabled
			if (thread_manager->_ping_broadcast) {
				thread_manager->_ping_broadcast_thread.emplace();
				std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // Wait some time before attempting to connect
			}

			// Attempt to connect
			auto connect_success = socket->connect(thread_manager->_addr, 1162);
			thread_manager->_ping_broadcast_thread.reset(); // Stop ping broadcast
			if (not connect_success) {
				LOG_FATAL("Connect failed", connect_success.error());
				return;
			}

			LOG_INFO("Connected");
			thread_manager->locked_set_state(pb::CLIENT_CONNECTED);
		} else {
			// Already connected

			// Prepare select
			fd_set read_set;
			FD_ZERO(&read_set);
			FD_SET(socket->fd(), &read_set);
			auto select_result = select(socket->fd(), &read_set, nullptr, 100);
			if (not select_result) {
				LOG_FATAL("Select failed", select_result.error());
				return;
			}

			// If any event has occurred on the fds
			if (0 < *select_result) {
				if (not FD_ISSET(socket->fd(), &read_set)) {
					// Skip if no messages have been received
					continue;
				}

				// Read message
				auto recv_success = socket->recv(thread_manager->_read_buffer, sizeof(thread_manager->_read_buffer) - 1);
				if (not recv_success) {
					LOG_ERROR("Receive failed", recv_success.error());
					continue;
				}
				// Check if connection dropped
				if (*recv_success == 0) {
					LOG_ERROR("Server disconnected");
					socket.reset();
					thread_manager->locked_set_state(pb::CLIENT_NOT_READY);
					continue;
				}

				// Process received bytes
				size_t processed_bytes = 0;
				while (processed_bytes < static_cast<size_t>(*recv_success)) {
					// Try parse
					std::string json_str{thread_manager->_read_buffer + processed_bytes};
					processed_bytes += json_str.size() + 1;
					auto expect_message = pb::json_string_to_message<pb::NetworkMessage>(json_str);
					if (not expect_message) {
						LOG_ERROR("Received bad message", expect_message.error(), json_str);
						continue;
					}
					if (expect_message->game_hash() != M2_GAME.hash()) {
						LOG_ERROR("Received message of unknown origin", json_str);
						continue;
					}

					if (expect_message->has_shutdown() && expect_message->shutdown()) {
						LOG_INFO("Client received shutdown message");
						thread_manager->locked_set_state(pb::CLIENT_SHUTDOWN);
						return;
					} else if (expect_message->has_server_update()) {
						LOG_INFO("Client received ServerUpdate", json_str);

						// Wait until the previous ServerUpdate is processed
						while (thread_manager->locked_has_server_update()) {
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
						}

						{
							const std::lock_guard lock(thread_manager->_mutex);
							thread_manager->_received_server_update = expect_message->server_update(); // TODO don't copy
							if (thread_manager->_state != pb::CLIENT_STARTED) {
								// Set the state as STARTED when the first ServerUpdate is received.
								thread_manager->unlocked_set_state(pb::CLIENT_STARTED);
							}
						}
					} else if (expect_message->has_server_command()) {
						LOG_INFO("Received ServerCommand", json_str);

						// Wait until the previous ServerCommand is processed
						while (thread_manager->locked_has_server_command()) {
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
						}

						{
							const std::lock_guard lock(thread_manager->_mutex);
							thread_manager->_received_server_command = expect_message->server_command(); // TODO don't copy
							if (thread_manager->_state != pb::CLIENT_STARTED) {
								// Set the state as STARTED when the first ServerUpdate is received.
								thread_manager->unlocked_set_state(pb::CLIENT_STARTED);
							}
						}
					} else {
						throw M2_ERROR("Unsupported message");
					}
				}
			}

			// Write outgoing messages
			size_t msg_count = thread_manager->locked_get_outgoing_message_count();
			for (size_t i = 0; i < msg_count; ++i) {
				auto msg = thread_manager->locked_unsafe_pop_outgoing_message();
				if (auto expect_json_str = pb::message_to_json_string(msg); expect_json_str) {
					LOG_DEBUG("Sending message...", *expect_json_str);
					// Send null character as a separator between messages
					auto send_success = socket->send(expect_json_str->c_str(), expect_json_str->size() + 1);
					if (not send_success) {
						LOG_ERROR("Send error", send_success.error());
					}
				}
			}
		}
	}
}
