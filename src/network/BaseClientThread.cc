#include <m2/network/BaseClientThread.h>
#include <m2/network/Socket.h>
#include <m2/network/Detail.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/Meta.h>
#include <unistd.h>
#include <m2/network/SocketManager.h>

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

void m2::network::detail::BaseClientThread::locked_set_ready(bool state) {
	LOG_INFO("Will send ready state", state);

	{
		const std::lock_guard lock(_mutex);
		pb::NetworkMessage msg;
		msg.set_game_hash(M2_GAME.hash());
		msg.set_ready(state);
		LOG_DEBUG("Readiness message queued");
		_outgoing_queue.push(std::move(msg));
	}

	auto locked_has_outgoing_message = [this]() {
		const std::lock_guard lock(_mutex);
		return not _outgoing_queue.empty();
	};

	while (locked_has_outgoing_message()) {
		// Wait until output queue is empty
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	locked_set_state(state ? pb::CLIENT_READY : pb::CLIENT_CONNECTED);
}

void m2::network::detail::BaseClientThread::locked_start_if_ready() {
	if (locked_get_client_state() == pb::CLIENT_READY) {
		locked_set_state(pb::CLIENT_STARTED);
	}
}

void m2::network::detail::BaseClientThread::locked_queue_client_command(const m2g::pb::ClientCommand& cmd) {
	INFO_FN();

	pb::NetworkMessage msg;
	msg.set_game_hash(M2_GAME.hash());
	msg.mutable_client_command()->CopyFrom(cmd);

	{
		const std::lock_guard lock(_mutex);
		_outgoing_queue.push(std::move(msg));
	}
}

void m2::network::detail::BaseClientThread::unlocked_set_state(pb::ClientState state) {
	LOG_DEBUG("Setting ClientThread state", pb::enum_name(state));
	_state = state;
}

void m2::network::detail::BaseClientThread::locked_set_state(pb::ClientState state) {
	const std::lock_guard lock(_mutex);
	unlocked_set_state(state);
}

void m2::network::detail::BaseClientThread::base_client_thread_func(BaseClientThread* thread_manager) {
	std::this_thread::sleep_for(std::chrono::seconds(1)); // Sleep one second to be sure that BaseClientThread is properly constructed. // TODO use cond_var
	// TODO detect the type of client and use the correct name for logging
	set_thread_name_for_logging("CL");
	LOG_INFO("BaseClientThread function");

	auto locked_should_continue_running = [](BaseClientThread* thread_manager) {
		auto current_state = thread_manager->locked_get_client_state();
		return current_state != pb::CLIENT_QUIT && current_state != pb::CLIENT_SHUTDOWN;
	};
	auto locked_has_outgoing_message = [](BaseClientThread* thread_manager) {
		const std::lock_guard lock(thread_manager->_mutex);
		return not thread_manager->_outgoing_queue.empty();
	};
	auto locked_has_incoming_message = [](BaseClientThread* thread_manager) {
		const std::lock_guard lock(thread_manager->_mutex);
		return not thread_manager->_incoming_queue.empty();
	};
	auto locked_has_unprocessed_server_update_or_command = [](BaseClientThread* thread_manager) {
		const std::lock_guard lock(thread_manager->_mutex);
		return thread_manager->_received_server_update || thread_manager->_received_server_command;
	};

	std::optional<SocketManager> socket_manager;
	while (locked_should_continue_running(thread_manager)) {
		if (auto state = thread_manager->locked_get_client_state(); state == pb::CLIENT_INITIAL_STATE || state == pb::CLIENT_RECONNECTING) {
			if (socket_manager) {
				throw M2_ERROR("Implementation error, unexpected socket");
			}

			// Not yet connected
			// TODO implement reconnection (clear all queues and buffers first)

			// Create socket
			auto expect_socket = Socket::create();
			if (not expect_socket) {
				LOG_FATAL("Socket creation failed", expect_socket.error());
				return;
			}
			socket_manager.emplace(std::move(*expect_socket), -1);
			LOG_INFO("Socket created");

			// Start ping broadcast if enabled
			if (thread_manager->_ping_broadcast) {
				thread_manager->_ping_broadcast_thread.emplace();
				std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // Wait some time before attempting to connect
			}

			// Attempt to connect
			auto connect_success = socket_manager->socket().connect(thread_manager->_addr, 1162);
			if (not connect_success) {
				throw M2_ERROR("Connect failed: " + connect_success.error());
			} else {
				if (not *connect_success) {
					// Wait some time and try again
					std::this_thread::sleep_for(std::chrono::seconds(1));
				} else {
					LOG_INFO("Client established connection");
					thread_manager->_ping_broadcast_thread.reset(); // Stop ping broadcast
					thread_manager->locked_set_state(pb::CLIENT_CONNECTED);
				}
			}
		} else {
			if (not socket_manager) {
				throw M2_ERROR("Implementation error, expected socket");
			}
			// Already connected, ready, or started

			// Wait until the previous ServerUpdate & ServerCommand is processed
			while (locked_has_unprocessed_server_update_or_command(thread_manager)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(250));
			}

			// Preprocess one incoming message
			if (locked_has_incoming_message(thread_manager)) {
				const std::lock_guard lock(thread_manager->_mutex);

				// Pop the front message
				auto front_message = std::move(thread_manager->_incoming_queue.front());
				thread_manager->_incoming_queue.pop();
				if (front_message.has_shutdown() && front_message.shutdown()) {
					LOG_INFO("Shutdown found in incoming queue");
					thread_manager->unlocked_set_state(pb::CLIENT_SHUTDOWN);
				} else if (front_message.has_server_update()) {
					if (thread_manager->_state == pb::CLIENT_CONNECTED) {
						LOG_WARN("ServerUpdate received while not ready, closing connection to server");
						// TODO maybe do not trust the server?
						socket_manager.reset();
						thread_manager->unlocked_set_state(pb::CLIENT_RECONNECTING);
					} else if (not thread_manager->_received_server_update) {
						// Ensure that the state is STARTED
						if (thread_manager->_state == pb::CLIENT_READY) {
							thread_manager->unlocked_set_state(pb::CLIENT_STARTED);
						}
						LOG_INFO("ServerUpdate found in incoming queue");
						auto* server_update = front_message.release_server_update();
						thread_manager->_received_server_update.emplace(std::move(*server_update));
						delete server_update;
					}
				} else if (front_message.has_server_command()) {
					if (thread_manager->_state != pb::CLIENT_STARTED) {
						LOG_WARN("ServerCommand received while not started, closing connection to server");
						// TODO maybe do not trust the server?
						socket_manager.reset();
						thread_manager->unlocked_set_state(pb::CLIENT_RECONNECTING);
					} else if (not thread_manager->_received_server_command) {
						LOG_INFO("ServerCommand found in incoming queue");
						auto* server_command = front_message.release_server_command();
						thread_manager->_received_server_command.emplace(std::move(*server_command));
						delete server_command;
					}
				} else {
					LOG_WARN("Unsupported message received from server, closing connection");
					// TODO maybe do not trust the server?
					socket_manager.reset();
					thread_manager->unlocked_set_state(pb::CLIENT_RECONNECTING);
				}
			}

			// Prepare sets for select
			fd_set read_set, write_set;
			FD_ZERO(&read_set); FD_ZERO(&write_set);
			FD_SET(socket_manager->socket().fd(), &read_set);
			if (socket_manager->has_outgoing_data() || locked_has_outgoing_message(thread_manager)) {
				FD_SET(socket_manager->socket().fd(), &write_set);
			}
			// Select
			auto select_result = select(socket_manager->socket().fd(), &read_set, &write_set, 250);
			if (not select_result) {
				throw M2_ERROR("Select failed: " + select_result.error());
			}
			if (*select_result == 0) {
				// Time out occurred
				continue;
			}

			// If there's anything to read
			if (FD_ISSET(socket_manager->socket().fd(), &read_set)) {
				const std::lock_guard lock(thread_manager->_mutex);
				auto read_result = socket_manager->read_incoming_data(thread_manager->_incoming_queue);
				if (not read_result) {
					LOG_WARN("Error occurred while reading, closing connection to server", read_result.error());
					socket_manager.reset();
					thread_manager->unlocked_set_state(pb::CLIENT_RECONNECTING);
					continue;
				} else if (*read_result != SocketManager::ReadResult::MESSAGE_RECEIVED && *read_result != SocketManager::ReadResult::INCOMPLETE_MESSAGE_RECEIVED) {
					LOG_WARN("Invalid data received from server, closing connection", static_cast<int>(*read_result));
					// TODO maybe do not trust the server?
					socket_manager.reset();
					thread_manager->unlocked_set_state(pb::CLIENT_RECONNECTING);
					continue;
				}
			}

			// If there's anything to write
			if (FD_ISSET(socket_manager->socket().fd(), &write_set)) {
				const std::lock_guard lock(thread_manager->_mutex);
				auto send_result = socket_manager->send_outgoing_data(thread_manager->_outgoing_queue);
				if (not send_result) {
					LOG_WARN("Error occurred while writing, closing connection to server", send_result.error());
					socket_manager.reset();
					thread_manager->unlocked_set_state(pb::CLIENT_RECONNECTING);
					continue;
				} else if (*send_result != SocketManager::SendResult::OK) {
					throw M2_ERROR("An invalid or too large outgoing message was queued to server: " + std::to_string(static_cast<int>(*send_result)));
				}
			}
		}
	}
}
