#include <m2/network/ServerThread.h>
#include <m2/network/Socket.h>
#include <m2/network/Detail.h>
#include <m2/network/FdSet.h>
#include <m2/protobuf/Detail.h>
#include <m2/ProxyDetail.h>
#include <m2/Log.h>
#include <m2/Meta.h>
#include <m2/Game.h>

#define PORT (1162)

m2::network::ServerThread::ServerThread(mplayer::Type type, unsigned max_connection_count) : _type(type),
	_max_connection_count(max_connection_count), _thread(ServerThread::thread_func, this) {
	DEBUG_FN();
}

m2::network::ServerThread::~ServerThread() {
	DEBUG_FN();
	set_state_locked(pb::ServerState::SERVER_QUIT);
	_thread.join();
}

bool m2::network::ServerThread::is_listening() {
	const std::lock_guard lock(_mutex);
	return _state == pb::ServerState::SERVER_LISTENING;
}

size_t m2::network::ServerThread::client_count() {
	const std::lock_guard lock(_mutex);
	return _clients.size();
}

int m2::network::ServerThread::turn_holder_index() {
	const std::lock_guard lock(_mutex);
	return _turn_holder_index;
}

std::optional<m2::pb::NetworkMessage> m2::network::ServerThread::pop_client_command() {
	const std::lock_guard lock(_mutex);
	if (_received_client_command) {
		LOG_DEBUG("Popping client command");
		auto tmp = std::move(_received_client_command);
		_received_client_command.reset();
		return tmp;
	}
	return std::nullopt;
}

bool m2::network::ServerThread::close_lobby() {
	{
		// Check if all clients reported as ready
		const std::lock_guard lock(_mutex);
		for (auto& client : _clients) {
			if (not client.sender_id) {
				LOG_INFO("Cannot close lobby because of unready clients");
				return false;
			}
		}
	}

	LOG_INFO("Closing lobby");
	set_state_locked(pb::ServerState::SERVER_READY);
	return true;
}

void m2::network::ServerThread::set_turn_holder_index(int idx) {
	LOG_INFO("New turn holder index", idx);
	const std::lock_guard lock(_mutex);
	_turn_holder_index = idx;
}

void m2::network::ServerThread::server_update() {
	DEBUG_FN();
	auto turn_holder = turn_holder_index();

	auto count = client_count();
	for (size_t i = 1; i < count; ++i) { // ServerUpdate is not sent to self
		QueueMessage qm;
		qm.destination = i;
		qm.message.set_game_hash(game_hash());
		qm.message.set_sender_id(0);
		qm.message.mutable_server_update()->set_receiver_index(I(i));
		qm.message.mutable_server_update()->set_turn_holder_index(turn_holder);
		for (auto player_id : m2g::multi_player_object_ids) {
			qm.message.mutable_server_update()->add_player_object_ids(player_id);
		}
		for (auto char_it : LEVEL.characters) { // Iterate over characters
			auto [char_variant, char_id] = char_it;
			auto* object_descriptor = qm.message.mutable_server_update()->add_objects_with_character();

			// For any Character type
			std::visit(overloaded {
					[object_descriptor](const auto& v) {
						object_descriptor->set_object_id(v.parent().id());
						object_descriptor->mutable_position()->CopyFrom(static_cast<pb::VecF>(v.parent().position));
						object_descriptor->set_object_type(v.parent().object_type());
						object_descriptor->set_parent_id(v.parent().parent_id());
						for (auto item_it = v.begin_items(); item_it != v.end_items(); ++item_it) {
							const auto* item_ptr = item_it.get();
							const auto* named_item_ptr = dynamic_cast<const NamedItem*>(item_ptr);
							if (!named_item_ptr) {
								throw M2FATAL("ServerUpdate does not support unnamed items");
							}
							object_descriptor->add_named_items(named_item_ptr->type());
						}
						pb::for_each_enum_value<m2g::pb::ResourceType>([&v, object_descriptor](m2g::pb::ResourceType rt) {
							if (v.has_resource(rt)) {
								auto* resource = object_descriptor->add_resources();
								resource->set_type(rt);
								resource->set_amount(v.get_resource(rt));
							}
						});
					}
			}, *char_variant);
		}

		queue_message_locked(std::move(qm));
	}
}

void m2::network::ServerThread::queue_server_command(int destination, const m2g::pb::ServerCommand& cmd) {
	DEBUG_FN();
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
	return _outgoing_message_queue.size();
}

void m2::network::ServerThread::set_state_locked(pb::ServerState state) {
	LOG_DEBUG("Setting new state", pb::enum_name(state));
	const std::lock_guard lock(_mutex);
	_state = state;
}

void m2::network::ServerThread::queue_message_unlocked(const QueueMessage& msg) {
	LOG_DEBUG("Queueing message to", msg.destination);
	_outgoing_message_queue.emplace_back(msg);
}

void m2::network::ServerThread::queue_message_unlocked(QueueMessage&& msg) {
	LOG_DEBUG("Queueing message to", msg.destination);
	_outgoing_message_queue.emplace_back(std::move(msg));
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
	DEBUG_FN();
	auto is_quit = [server_thread]() {
		const std::lock_guard lock(server_thread->_mutex);
		return server_thread->_state == pb::ServerState::SERVER_QUIT;
	};
	auto pop_message = [server_thread]() -> std::optional<QueueMessage> {
		const std::lock_guard lock(server_thread->_mutex);
		if (!server_thread->_outgoing_message_queue.empty()) {
			LOG_DEBUG("Popping message from outgoing queue");
			auto tmp = std::move(server_thread->_outgoing_message_queue.front());
			server_thread->_outgoing_message_queue.pop_front();
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
					LOG_DEBUG("Will send message", msg.destination, *expect_json_str);
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
	LOG_DEBUG("Socket created");

	auto bind_success = listen_socket->bind(PORT);
	if (not bind_success) {
		LOG_FATAL("Bind failed", bind_success.error());
		return;
	}
	LOG_DEBUG("Socket binded");

	auto listen_success = listen_socket->listen(I(server_thread->_max_connection_count));
	if (not listen_success) {
		LOG_FATAL("Listen failed", listen_success.error());
		return;
	}
	LOG_INFO("Socket listening on port", PORT);
	server_thread->set_state_locked(pb::ServerState::SERVER_LISTENING);

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
				LOG_DEBUG("Accepted new connection");

				if (server_thread->_max_connection_count <= server_thread->_clients.size()) {
					// Reject new connection, do not store client socket
					LOG_DEBUG("Closing connection because of connection limit");
				} else {
					read_set.add_fd(client_socket->fd()); // Add client to read set
					auto client = Client{std::move(*client_socket), std::nullopt};
					server_thread->_clients.emplace_back(std::move(client));
					LOG_INFO("New client connected", server_thread->_clients.size() - 1);
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
					LOG_ERROR("Received bad message", i, json_str);
					continue;
				}

				if (expect_message->game_hash() != game_hash()) {
					LOG_ERROR("Received message of unknown origin", i, json_str);
					continue;
				}

				if (not expect_message->has_client_command() && not expect_message->has_client_update()) {
					LOG_INFO("Received ping", i);
					if (auto sender_id = expect_message->sender_id(); sender_id) {
						if (not client.sender_id) {
							client.sender_id = sender_id;
							LOG_INFO("Client ready", i, sender_id);
						} else if (client.sender_id != expect_message->sender_id()) {
							// TODO handle edge case
						}
					} else {
						LOG_INFO("Client not ready", i, *client.sender_id);
						client.sender_id = std::nullopt;
					}
				} else if (expect_message->has_client_command()) {
					if (I(i) == server_thread->_turn_holder_index && not server_thread->_received_client_command) {
						LOG_INFO("Recieved ClientCommand", i, json_str);
						server_thread->_received_client_command = *expect_message;
					}
				} else {
					// TODO process other incoming messages
				}
			}
			if (server_thread->_state == pb::ServerState::SERVER_LISTENING) {
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
