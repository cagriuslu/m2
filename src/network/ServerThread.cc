#include <m2/network/ServerThread.h>
#include <m2/network/Socket.h>
#include <m2/network/Detail.h>
#include <m2/protobuf/Detail.h>
#include <m2/Log.h>
#include <m2/Meta.h>
#include <m2/Game.h>

#define PORT (1162)

m2::network::ServerThread::ServerThread(mplayer::Type type, unsigned max_connection_count) : _type(type),
	_max_connection_count(max_connection_count), _thread(ServerThread::thread_func, this) {}

m2::network::ServerThread::~ServerThread() {
	DEBUG_FN();
	set_state_locked(pb::ServerState::SERVER_QUIT);
	if (_thread.joinable()) {
		// If the object is default created, thread may not be joinable
		_thread.join();
	}
}

bool m2::network::ServerThread::is_listening() {
	const std::lock_guard lock(_mutex);
	return _state == pb::ServerState::SERVER_LISTENING;
}

int m2::network::ServerThread::client_count() {
	const std::lock_guard lock(_mutex);
	return I(_clients.size());
}

int m2::network::ServerThread::ready_client_count() {
	const std::lock_guard lock(_mutex);
	int sum = 0;
	for (const auto& client: _clients) {
		if (client.is_ready()) {
			++sum;
		}
	}
	return sum;
}

int m2::network::ServerThread::turn_holder_index() {
	const std::lock_guard lock(_mutex);
	return _turn_holder;
}

std::optional<m2::pb::NetworkMessage> m2::network::ServerThread::pop_turn_holder_command() {
	TRACE_FN();
	const std::lock_guard lock(_mutex);
	auto opt_message = _clients[_turn_holder].pop_incoming_message();
	if (opt_message) {
		auto json_str = pb::message_to_json_string(*opt_message);
		LOG_DEBUG("Popping client command", _turn_holder, json_str->c_str());
	}
	return opt_message;
}

bool m2::network::ServerThread::is_shutdown() {
	const std::lock_guard lock(_mutex);
	return _state == pb::ServerState::SERVER_SHUTDOWN;
}

m2::void_expected m2::network::ServerThread::close_lobby() {
	LOG_INFO("Closing lobby...");
	{
		// Check if all clients are ready
		const std::lock_guard lock(_mutex);
		if (not std::ranges::all_of(_clients, server::is_client_ready)) {
			return make_unexpected("Not every client is ready");
		}
		// Stop ping broadcast
		_ping_broadcast_thread.reset();
		set_state_unlocked(pb::ServerState::SERVER_READY);
	}
	LOG_INFO("Lobby closed");
	return {};
}

void m2::network::ServerThread::set_turn_holder(int idx) {
	LOG_INFO("New turn holder index", idx);
	const std::lock_guard lock(_mutex);
	_turn_holder = idx;
}

void m2::network::ServerThread::send_server_update() {
	INFO_FN();

	// Prepare the ServerUpdate except the receiver_index field
	pb::NetworkMessage message;
	message.set_game_hash(M2_GAME.hash());
	message.mutable_server_update()->set_turn_holder_index(turn_holder_index());
	for (auto player_id : M2G_PROXY.multi_player_object_ids) {
		message.mutable_server_update()->add_player_object_ids(player_id);
	}
	for (auto& char_variant : M2_LEVEL.characters) { // Iterate over characters
		auto* object_descriptor = message.mutable_server_update()->add_objects_with_character();

		// For any Character type
		std::visit(overloaded {
			[object_descriptor](const auto& v) {
				object_descriptor->set_object_id(v.owner().id());
				object_descriptor->mutable_position()->CopyFrom(static_cast<pb::VecF>(v.owner().position));
				object_descriptor->set_object_type(v.owner().object_type());
				object_descriptor->set_parent_id(v.owner().parent_id());
				for (auto item_it = v.begin_items(); item_it != v.end_items(); ++item_it) {
					const auto* item_ptr = item_it.get();
					const auto* named_item_ptr = dynamic_cast<const NamedItem*>(item_ptr);
					if (!named_item_ptr) {
						throw M2_ERROR("ServerUpdate does not support unnamed items");
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
				pb::for_each_enum_value<m2g::pb::AttributeType>([&v, object_descriptor](m2g::pb::AttributeType at) {
					if (v.has_attribute(at)) {
						auto* attribute = object_descriptor->add_attributes();
						attribute->set_type(at);
						attribute->set_amount(v.get_attribute(at));
					}
				});
			}
		}, char_variant);
	}

	// Send to clients.
	auto count = client_count();
	for (auto i = 1; i < count; ++i) { // ServerUpdate is not sent to self
		const std::lock_guard lock(_mutex);

		// Make sure the state is set as READY
		if (_state != pb::SERVER_READY) {
			set_state_unlocked(pb::SERVER_READY);
		}

		LOG_DEBUG("Queueing ServerUpdate to client", i);
		message.mutable_server_update()->set_receiver_index(i);
		_clients[i].queue_outgoing_message(message);
	}
}

void m2::network::ServerThread::send_server_command(const m2g::pb::ServerCommand& command, int receiver_index) {
	LOG_INFO("Sending server command to index", receiver_index);

	if (receiver_index < 0 || client_count() <= receiver_index) {
		throw M2_ERROR("Client index not found");
	}

	pb::NetworkMessage message;
	message.set_game_hash(M2_GAME.hash());
	message.mutable_server_command()->CopyFrom(command);

	{
		const std::lock_guard lock(_mutex);
		LOG_DEBUG("Queueing ServerCommand to client", receiver_index);
		_clients[receiver_index].queue_outgoing_message(std::move(message));
	}
}

void m2::network::ServerThread::shutdown() {
	LOG_INFO("Shutting down the server");
	const std::lock_guard lock(_mutex);

	pb::NetworkMessage msg;
	msg.set_game_hash(M2_GAME.hash());
	msg.set_shutdown(true);

	// Send to clients
	auto count = I(_clients.size());
	for (auto i = 0; i < count; ++i) {
		LOG_DEBUG("Queueing Shutdown message to client", i);
		_clients[i].queue_outgoing_message(msg);
	}
	// Flush output queues
	for (auto i = 0; i < count; ++i) {
		_clients[i].flush_outgoing_messages();
	}

	set_state_unlocked(pb::SERVER_SHUTDOWN);
}

void m2::network::ServerThread::set_state_locked(pb::ServerState state) {
	const std::lock_guard lock(_mutex);
	set_state_unlocked(state);
}

void m2::network::ServerThread::set_state_unlocked(pb::ServerState state) {
	LOG_DEBUG("Setting state", pb::ServerState_Name(state));
	_state = state;
}

void m2::network::ServerThread::thread_func(ServerThread* server_thread) {
	set_thread_name_for_logging("SR");
	LOG_INFO("ServerThread function");

	auto listen_socket = Socket::create_tcp();
	if (not listen_socket) {
		LOG_FATAL("Socket creation failed", listen_socket.error());
		return;
	}
	LOG_DEBUG("Socket created");

	// Try binding to the socket multiple times.
	bool binded = false;
	m2_repeat(32) { // The socket may linger up to 30 secs
		auto bind_result = listen_socket->bind(PORT);
		if (not bind_result) {
			LOG_FATAL("Bind failed", bind_result.error());
			return;
		}
		if (not *bind_result) {
			LOG_INFO("Socket is busy, retry binding");
			m2::sdl::delay(1000);
		} else {
			binded = true;
			break;
		}
	}
	if (not binded) {
		LOG_FATAL("Bind failed: Address already in use");
		abort(); // TODO remove later. Eases development.
		return;
	}
	LOG_DEBUG("Socket bound");

	auto listen_success = listen_socket->listen(I(server_thread->_max_connection_count));
	if (not listen_success) {
		LOG_FATAL("Listen failed", listen_success.error());
		return;
	}
	LOG_INFO("Socket listening on port", PORT);
	server_thread->set_state_locked(pb::ServerState::SERVER_LISTENING);

	// Start ping broadcast
	server_thread->_ping_broadcast_thread.emplace();

	while (not server_thread->is_quit() && not server_thread->is_shutdown()) {
		fd_set read_set;
		auto max_fd = server_thread->prepare_read_set(&read_set);
		FD_SET(listen_socket->fd(), &read_set); // Add listen socket as well
		max_fd = std::max(max_fd, listen_socket->fd());

		auto select_result = select(max_fd, &read_set, nullptr, 100);
		if (not select_result) {
			LOG_FATAL("Select failed", select_result.error());
			server_thread->_clients.clear();
			return;
		}

		if (0 < *select_result) { // If select haven't timed-out
			// Check main socket
			if (FD_ISSET(listen_socket->fd(), &read_set)) {
				LOG_TRACE("Main socket is readable");

				auto client_socket = listen_socket->accept();
				if (not client_socket) {
					LOG_FATAL("Accept failed", select_result.error());
					server_thread->_clients.clear();
					return;
				}
				if (server_thread->_max_connection_count <= server_thread->_clients.size()) {
					// Reject new connection, do not store client socket
					LOG_DEBUG("Closing connection because of connection limit");
				} else {
					LOG_INFO("New client connected with index", server_thread->_clients.size());
					server_thread->_clients.emplace_back(std::move(*client_socket));
				}
			}

			// Check clients
			{
				const std::lock_guard lock(server_thread->_mutex);
				for (size_t i = 0; i < server_thread->_clients.size(); ++i) {
					auto& client = server_thread->_clients[i];
					if (not client.is_still_connected()) {
						// Skip client if it's connection has dropped
						continue;
					}
					if (not FD_ISSET(client.socket().fd(), &read_set)) {
						// Skip if no messages have been received from this client
						continue;
					}
					LOG_DEBUG("Client socket with index is readable", i);

					// The size of the buffer is passed as one character short, so that there's always a null at the end
					if (auto expect_new_message = client.save_incoming_message(server_thread->_read_buffer, sizeof(server_thread->_read_buffer) - 1); not expect_new_message) {
						LOG_ERROR("Failed to save message", expect_new_message.error());
						server_thread->_clients.clear();
						return;
					} else {
						if (auto is_new_message_received = *expect_new_message; not is_new_message_received) {
							// A soft error occurred
							// Client might still have an unprocessed incoming message,
							// or the client connection might have dropped.
							// These errors should not cause the server to shut down.
						} else if (client.peak_incoming_message()->has_ready()) {
							// Check ready message
							if (server_thread->_state == pb::SERVER_LISTENING) {
								LOG_INFO("Client state", client.peak_incoming_message()->ready());
								client.set_ready(client.peak_incoming_message()->ready());
							} else {
								LOG_WARN("Received ready signal while the server wasn't listening");
							}
							client.pop_incoming_message(); // Pop the message from the client
						} else {
							// Process other messages
							if (server_thread->_turn_holder != I(i)) {
								LOG_WARN("Dropping message received from a non-turn-holder client", i);
								client.pop_incoming_message();
							} else {
								// Process ClientCommand
								if (client.peak_incoming_message()->has_client_command()) {
									auto json_str = pb::message_to_json_string(*client.peak_incoming_message());
									LOG_DEBUG("Received ClientCommand", i, *json_str);
									// Processing of the message will be done by the game loop, don't pop it.
								} else {
									// TODO process other client messages
									client.pop_incoming_message(); // TEMP
								}
							}
						}
					}
				}

				// If the lobby is not yet closed, remove disconnected clients
				if (server_thread->_state == pb::ServerState::SERVER_LISTENING) {
					auto erase_it = std::remove_if(server_thread->_clients.begin(), server_thread->_clients.end(), [](const auto& client) {
						return not client.is_still_connected();
					});
					server_thread->_clients.erase(erase_it, server_thread->_clients.end());
				}
			}
		}

		// Write outgoing messages
		{
			const std::lock_guard lock(server_thread->_mutex);
			for (size_t i = 0; i < server_thread->_clients.size(); ++i) {
				auto& client = server_thread->_clients[i];
				auto flush_success = client.flush_outgoing_messages();
				if (not flush_success) {
					LOG_ERROR("Failed to send message", flush_success.error());
					server_thread->_clients.clear();
					return;
				} else if (*flush_success) {
					LOG_DEBUG("Message sent to client", i);
				}
			}
		}
	}

	LOG_INFO("Server thread is quiting");
}

bool m2::network::ServerThread::is_quit() {
	const std::lock_guard lock(_mutex);
	return _state == pb::ServerState::SERVER_QUIT;
}

int m2::network::ServerThread::prepare_read_set(fd_set* set) {
	FD_ZERO(set);

	const std::lock_guard lock(_mutex);
	int max = 0;
	for (auto& client : _clients) {
		if (client.is_still_connected()) {
			auto fd = client.socket().fd();
			FD_SET(fd, set);
			max = std::max(max, fd);
		}
	}
	return max;
}
