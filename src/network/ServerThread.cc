#include <m2/network/ServerThread.h>
#include <m2/network/Socket.h>
#include <m2/network/Detail.h>
#include <m2/network/FdSet.h>
#include <m2/protobuf/Detail.h>
#include <m2/ProxyDetail.h>
#include <m2/Log.h>
#include <m2/Meta.h>

#define PORT (1162)

m2::network::ServerThread::ServerThread(mplayer::Type type, unsigned max_connection_count) : _type(type),
	_max_connection_count(max_connection_count), _thread(ServerThread::thread_func, this) {}

m2::network::ServerThread::~ServerThread() {
	set_state_locked(State::Quit);
	_thread.join();
}

bool m2::network::ServerThread::is_listening() {
	const std::lock_guard lock(_mutex);
	return _state == State::Listening;
}

size_t m2::network::ServerThread::client_count() {
	const std::lock_guard lock(_mutex);
	return _clients.size();
}

bool m2::network::ServerThread::close_lobby() {
	{
		// Check if all clients reported as ready
		const std::lock_guard lock(_mutex);
		for (auto& client : _clients) {
			if (not client.sender_id) {
				return false;
			}
		}
	}

	set_state_locked(State::Ready);
	return true;
}

void m2::network::ServerThread::server_update() {
	auto count = client_count();
	for (size_t i = 1; i < count; ++i) { // Server update is not sent to self
		QueueMessage qm;
		qm.destination = i;
		qm.message.set_game_hash(game_hash());
		qm.message.set_sender_id(0);
		qm.message.mutable_server_update()->set_receiver_index(I(i));
		for (auto player_id : m2g::multi_player_object_ids) {
			qm.message.mutable_server_update()->add_player_object_ids(player_id);
		}
		// TODO

		queue_message_locked(std::move(qm));
	}
}

void m2::network::ServerThread::queue_server_command(int destination, const m2g::pb::ServerCommand& cmd) {
	QueueMessage que_msg;
	que_msg.message.set_game_hash(game_hash());
	que_msg.message.set_sender_id(0);
	que_msg.message.mutable_server_command()->CopyFrom(cmd);

	const std::lock_guard lock(_mutex);
	if (destination == -1) {
		for (unsigned i = 0; i < _clients.size(); ++i) {
			que_msg.destination = i;
			queue_message_unlocked(que_msg);
		}
	} else {
		que_msg.destination = destination;
		queue_message_unlocked(que_msg);
	}
}

size_t m2::network::ServerThread::message_count_locked() {
	const std::lock_guard lock(_mutex);
	return _message_queue.size();
}

void m2::network::ServerThread::set_state_locked(State state) {
	const std::lock_guard lock(_mutex);
	_state = state;
}

void m2::network::ServerThread::queue_message_unlocked(const QueueMessage& msg) {
	_message_queue.emplace_back(msg);
}

void m2::network::ServerThread::queue_message_unlocked(QueueMessage&& msg) {
	_message_queue.emplace_back(std::move(msg));
}

void m2::network::ServerThread::queue_message_locked(const QueueMessage& msg) {
	const std::lock_guard lock(_mutex);
	queue_message_unlocked(msg);
}

void m2::network::ServerThread::queue_message_locked(QueueMessage&& msg) {
	const std::lock_guard lock(_mutex);
	queue_message_unlocked(std::move(msg));
}

void m2::network::ServerThread::thread_func(ServerThread* server_thread) {
	auto is_quit = [server_thread]() {
		const std::lock_guard lock(server_thread->_mutex);
		return server_thread->_state == State::Quit;
	};
	auto pop_message = [server_thread]() -> std::optional<QueueMessage> {
		const std::lock_guard lock(server_thread->_mutex);
		if (!server_thread->_message_queue.empty()) {
			auto tmp = std::move(server_thread->_message_queue.front());
			server_thread->_message_queue.pop_front();
			return std::move(tmp);
		} else {
			return std::nullopt;
		}
	};
	auto send_message_locked = [server_thread](const QueueMessage& msg) {
		const std::lock_guard lock(server_thread->_mutex);
		if (msg.destination < server_thread->_clients.size()) {
			if (server_thread->_clients[msg.destination].socket) {
				if (auto expect_json_str = pb::message_to_json_string(msg.message); expect_json_str) {
					auto send_success = server_thread->_clients[msg.destination].socket->send(expect_json_str->data(), expect_json_str->size());
					if (not send_success) {
						LOG_ERROR("Send error", send_success.error());
					}
				}
			}
		}
	};

	auto listen_socket = Socket::create_tcp();
	if (not listen_socket) {
		LOG_FATAL("Socket creation failed", listen_socket.error());
		return;
	}

	auto bind_success = listen_socket->bind(PORT);
	if (not bind_success) {
		LOG_FATAL("Bind failed", bind_success.error());
		return;
	}

	auto listen_success = listen_socket->listen(I(server_thread->_max_connection_count));
	if (not listen_success) {
		LOG_FATAL("Listen failed", listen_success.error());
		return;
	}
	server_thread->set_state_locked(State::Listening);
	LOG_INFO("Listening on port", PORT);

	FdSet read_set, write_set;
	read_set.add_fd(listen_socket->fd()); // Add listen socket

	while (!is_quit()) {
		auto select_result = select(read_set, write_set, 100);
		if (not select_result) {
			LOG_FATAL("Select failed", select_result.error());
			server_thread->_clients.clear();
			return;
		}

		if (*select_result == 0) {
			// Timeout
			LOG_TRACE("Select timeout");
		} else {
			// Check main socket
			if (read_set.is_set(listen_socket->fd())) {
				auto client_socket = listen_socket->accept();
				if (not client_socket) {
					LOG_FATAL("Accept failed", select_result.error());
					server_thread->_clients.clear();
					return;
				}

				if (server_thread->_max_connection_count <= server_thread->_clients.size()) {
					// Reject new connection, do not store client socket
				} else {
					read_set.add_fd(client_socket->fd()); // Add client to read set
					auto client = Client{std::move(*client_socket), std::nullopt};
					server_thread->_clients.emplace_back(std::move(client));
					LOG_INFO("Client connected");
				}
			}

			// Check clients
			const std::lock_guard lock(server_thread->_mutex);
			for (size_t i = 0; i < server_thread->_clients.size(); ++i) {
				auto& client = server_thread->_clients[i];
				if (not client.socket) {
					// Skip client if it's connection has dropped
					continue;
				}

				if (not read_set.is_set(client.socket->fd())) {
					// Skip if no messages have been received from this client
					continue;
				}

				auto recv_success = client.socket->recv(server_thread->_read_buffer, sizeof(server_thread->_read_buffer));
				if (not recv_success) {
					LOG_ERROR("Receive failed", recv_success.error());
					continue;
				}
				if (*recv_success == 0) {
					LOG_ERROR("Client disconnected");
					read_set.remove_fd(client.socket->fd());
					client.socket.reset(); // Close socket from our side as well
					continue;
				}

				// Parse message
				std::string json_str{server_thread->_read_buffer, static_cast<size_t>(*recv_success)};
				auto expect_message = pb::json_string_to_message<pb::NetworkMessage>(json_str);
				if (not expect_message) {
					LOG_ERROR("Received bad message", json_str);
					continue;
				}

				if (expect_message->game_hash() != game_hash()) {
					LOG_ERROR("Received message of unknown origin", json_str);
					continue;
				}

				if (not expect_message->has_client_command() && not expect_message->has_client_update()) {
					LOG_INFO("Received ping");
					if (auto sender_id = expect_message->sender_id(); sender_id) {
						if (not client.sender_id) {
							client.sender_id = sender_id;
							LOG_INFO("Client ready", sender_id);
						} else if (client.sender_id != expect_message->sender_id()) {
							// TODO handle edge case
						}
					} else {
						LOG_INFO("Client not ready", *client.sender_id);
						client.sender_id = std::nullopt;
					}
				} else {
					// TODO process other incoming messages
				}
			}
			if (server_thread->_state == State::Listening) {
				auto erase_it = std::remove_if(server_thread->_clients.begin(), server_thread->_clients.end(), [](const auto& client) {
					return not client.socket.has_value();
				});
				server_thread->_clients.erase(erase_it, server_thread->_clients.end());
			}
		}

		// Write outgoing messages
		size_t msg_count = server_thread->message_count_locked();
		for (size_t i = 0; i < msg_count; ++i) {
			auto expect_msg = pop_message();
			if (not expect_msg) {
				break; // No message left in queue
			}
			send_message_locked(*expect_msg);
		}
	}
}
