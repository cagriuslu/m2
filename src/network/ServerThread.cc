#include <m2/network/ServerThread.h>
#include <m2/network/TcpSocket.h>
#include <m2/network/Detail.h>
#include <m2/protobuf/Detail.h>
#include <m2/Log.h>
#include <m2/Meta.h>
#include <m2/Game.h>
#include <algorithm>

#define PORT (1162)

m2::network::ServerThread::ServerThread(mplayer::Type type, unsigned max_connection_count) : _type(type),
	_max_connection_count(max_connection_count), _thread(ServerThread::thread_func, this) {
	_latch.count_down();
}

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
	return I(std::ranges::count_if(_clients, is_client_ready));
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

bool m2::network::ServerThread::has_reconnected_client() {
	const std::lock_guard lock(_mutex);
	return _has_reconnected_client;
}
std::optional<int> m2::network::ServerThread::disconnected_client() {
	const std::lock_guard lock(_mutex);
	for (auto i = 0; i < I(_clients.size()); ++i) {
		if (auto disconnected_since = _clients[i].disconnected_or_untrusted_since();
			disconnected_since && *disconnected_since + 15000 < sdl::get_ticks()) {
			return i;
		}
	}
	return std::nullopt;
}

std::optional<int> m2::network::ServerThread::misbehaved_client() {
	const std::lock_guard lock(_mutex);
	for (auto i = 0; i < I(_clients.size()); ++i) {
		if (_clients[i].has_misbehaved()) {
			return i;
		}
	}
	return std::nullopt;
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
		if (not std::ranges::all_of(_clients, is_client_ready)) {
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

m2::pb::NetworkMessage m2::network::ServerThread::prepare_server_update() {
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

	return message;
}
void m2::network::ServerThread::send_server_update() {
	INFO_FN();

	// Make sure the state is set as READY
	if (_state != pb::SERVER_READY) {
		set_state_unlocked(pb::SERVER_READY);
	}

	pb::NetworkMessage message = prepare_server_update();
	{
		const std::lock_guard lock(_mutex);
		// Send to clients
		auto count = client_count();
		for (auto i = 1; i < count; ++i) { // ServerUpdate is not sent to self
			if (_clients[i].is_ready()) {
				LOG_DEBUG("Queueing ServerUpdate to client", i);
				message.mutable_server_update()->set_receiver_index(i);
				_clients[i].queue_outgoing_message(message);
			}
		}
		// Clear reconnected client
		_has_reconnected_client = false;
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
		if (_clients[receiver_index].is_ready()) {
			LOG_DEBUG("Queueing ServerCommand to client", receiver_index);
			_clients[receiver_index].queue_outgoing_message(std::move(message));
		} else {
			LOG_WARN("Attempted to queue ServerCommand but client is disconnected");
		}
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
		if (_clients[i].is_ready()) {
			LOG_DEBUG("Queueing Shutdown message to client", i);
			_clients[i].queue_outgoing_message(msg);
		}
	}
	// Flush output queues
	for (auto i = 0; i < count; ++i) {
		_clients[i].flush_and_shutdown();
	}

	set_state_unlocked(pb::SERVER_SHUTDOWN);
}

m2::pb::ServerState m2::network::ServerThread::locked_get_state() {
	const std::lock_guard lock(_mutex);
	return _state;
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
	server_thread->_latch.wait();
	set_thread_name_for_logging("SR");
	LOG_INFO("ServerThread function");

	auto listen_socket = TcpSocket::create();
	if (not listen_socket) {
		throw M2_ERROR("TcpSocket creation failed: " + listen_socket.error());
	}
	LOG_DEBUG("TcpSocket created");

	// Try binding to the socket multiple times.
	bool binded = false;
	m2_repeat(32) { // The socket may linger up to 30 secs
		auto bind_result = listen_socket->bind(PORT);
		if (not bind_result) {
			throw M2_ERROR("Bind failed: " + bind_result.error());
		}
		if (not *bind_result) {
			LOG_INFO("TcpSocket is busy, retry binding");
			m2::sdl::delay(1000);
		} else {
			binded = true;
			break;
		}
	}
	if (not binded) {
		throw M2_ERROR("Bind failed: Address already in use");
	}
	LOG_DEBUG("TcpSocket bound");

	auto listen_success = listen_socket->listen(I(server_thread->_max_connection_count));
	if (not listen_success) {
		throw M2_ERROR("Listen failed: " + listen_success.error());
	}
	LOG_INFO("TcpSocket listening on port", PORT);
	server_thread->set_state_locked(pb::ServerState::SERVER_LISTENING);

	// Start ping broadcast
	server_thread->_ping_broadcast_thread.emplace();

	while (not server_thread->locked_is_quit() && not server_thread->is_shutdown()) {
		// Process one incoming message from each client
		{
			const std::lock_guard lock(server_thread->_mutex);
			for (auto i = 0; i < I(server_thread->_clients.size()); ++i) {
				auto& client = server_thread->_clients[i];
				if (client.has_incoming_data(false)) {
					if (const auto* peak = client.peak_incoming_message(); peak->has_client_update()) {
						if (server_thread->_state == pb::SERVER_LISTENING) {
							LOG_INFO("Received client ready token", i, peak->client_update().ready_token());
							client.set_ready_token(peak->client_update().ready_token()); // TODO handle response
						} else if (server_thread->_state == pb::SERVER_STARTED && client.is_untrusted()) {
							if (client.set_ready_token(peak->client_update().ready_token())) {
								LOG_INFO("Previously reconnected client has presented the correct ready token, will send ServerUpdate", i);
								server_thread->_has_reconnected_client = true;
							} else {
								LOG_INFO("Previously reconnected client presented incorrect ready token, disconnecting client", i);
								client.honorably_disconnect();
							}
						} else {
							LOG_WARN("Received unexpected ClientUpdate", i);
							client.set_misbehaved();
						}
						client.pop_incoming_message(); // Message handled
					} else if (peak->has_client_command()) {
						if (server_thread->_turn_holder != i) {
							LOG_WARN("Received ClientCommand from a non-turn-holder client", i);
							client.set_misbehaved();
							client.pop_incoming_message(); // Message handled
						} else {
							LOG_INFO("ClientCommand is received, will be processed by game loop", i);
						}
					} else {
						LOG_WARN("Received unexpected message from client", i);
						client.set_misbehaved();
					}
				}
			}
		}

		// Prepare read set for select
		fd_set read_set;
		auto read_max_fd = server_thread->prepare_read_fd_set(&read_set);
		FD_SET(listen_socket->fd(), &read_set); // Add main socket as well
		read_max_fd = std::max(read_max_fd, listen_socket->fd());
		// Prepare write set for select
		fd_set write_set;
		auto write_max_fd = server_thread->prepare_write_fd_set(&write_set);
		// Select
		auto select_result = select(std::max(read_max_fd, write_max_fd), &read_set, &write_set, 250);
		if (not select_result) {
			throw M2_ERROR("Select failed: " + select_result.error());
		}
		if (*select_result == 0) {
			// Time out occurred
			continue;
		}

		// Check main socket
		if (FD_ISSET(listen_socket->fd(), &read_set)) {
			LOG_INFO("Main socket is readable");

			auto client_socket = listen_socket->accept();
			if (not client_socket) {
				throw M2_ERROR("Accept failed: " + client_socket.error());
			} else if (not client_socket->has_value()) {
				LOG_WARN("Client aborted connection by the time it was accepted");
			} else {
				if (server_thread->_max_connection_count <= server_thread->_clients.size()) {
					LOG_INFO("Refusing connection because of connection limit", (*client_socket)->address_and_port());
				} else if (auto state = server_thread->locked_get_state(); state == pb::SERVER_LISTENING) {
					LOG_INFO("New client connected with index", server_thread->_clients.size(), (*client_socket)->address_and_port());
					server_thread->_clients.emplace_back(std::move(**client_socket), server_thread->_clients.size());
				} else if (state == pb::SERVER_READY) {
					LOG_INFO("Refusing connection to closed lobby", (*client_socket)->address_and_port());
				} else if (state == pb::SERVER_STARTED) {
					const std::lock_guard lock(server_thread->_mutex);
					// Check if there's a disconnected client
					bool found = false;
					for (int i = 0; i < I(server_thread->_clients.size()) && not found; ++i) {
						auto& client = server_thread->_clients[i];
						if (client.is_disconnected() && client.address_and_port() == (*client_socket)->address_and_port()) {
							LOG_INFO("Previously connected client with index connected again", i, (*client_socket)->address_and_port());
							client.untrusted_client_reconnected(std::move(**client_socket));
							found = true;
						}
					}
					if (not found) {
						LOG_INFO("Refusing connection after game started", (*client_socket)->address_and_port());
					}
				} else {
					throw M2_ERROR("Unexpected state");
				}
			}
		}

		// Check readable clients
		{
			const std::lock_guard lock(server_thread->_mutex);
			for (auto i = 0; i < I(server_thread->_clients.size()); ++i) {
				auto& client = server_thread->_clients[i];
				if (not client.is_connected()) {
					// Skip client if it's connection has dropped
					continue;
				}
				// Read from socket
				client.has_incoming_data(FD_ISSET(client.fd(), &read_set));
			}

			// If the lobby is not yet closed, remove disconnected clients
			if (server_thread->_state == pb::ServerState::SERVER_LISTENING) {
				auto erase_it = std::remove_if(server_thread->_clients.begin(), server_thread->_clients.end(),
					[](auto& client) { return client.is_disconnected_or_untrusted(); });
				server_thread->_clients.erase(erase_it, server_thread->_clients.end());
			}
		}
		// Give a breather to the mutex

		// Check writeable clients
		{
			const std::lock_guard lock(server_thread->_mutex);
			for (auto & client : server_thread->_clients) {
				if (not client.is_ready() || not FD_ISSET(client.fd(), &write_set)) {
					// Skip if the connection has dropped or the socket is not writeable
					continue;
				}
				// Write to socket
				client.send_outgoing_data();
			}
		}
	}

	LOG_INFO("Server thread is quiting");
}

bool m2::network::ServerThread::locked_is_quit() {
	const std::lock_guard lock(_mutex);
	return _state == pb::ServerState::SERVER_QUIT;
}

int m2::network::ServerThread::prepare_read_fd_set(fd_set* set) {
	FD_ZERO(set);

	const std::lock_guard lock(_mutex);
	int max = 0;
	for (auto& client : _clients) {
		if (client.is_connected()) {
			auto fd = client.fd();
			FD_SET(fd, set);
			max = std::max(max, fd);
		}
	}
	return max;
}

int m2::network::ServerThread::prepare_write_fd_set(fd_set* set) {
	FD_ZERO(set);

	const std::lock_guard lock(_mutex);
	int max = 0;
	for (auto& client : _clients) {
		if (client.is_ready() && client.has_outgoing_data()) {
			auto fd = client.fd();
			FD_SET(fd, set);
			max = std::max(max, fd);
		}
	}
	return max;
}
