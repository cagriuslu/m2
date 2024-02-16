#include <m2/network/ServerThread.h>
#include <m2/network/Socket.h>
#include <m2/network/Detail.h>
#include <m2/protobuf/Detail.h>
#include <m2/Log.h>
#include <m2/Meta.h>
#include <m2/Game.h>

#define PORT (1162)

m2::network::ServerThread::ServerThread(mplayer::Type type, unsigned max_connection_count) : _type(type),
	_max_connection_count(max_connection_count), _thread(ServerThread::thread_func, this) {
	INFO_FN();
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

int m2::network::ServerThread::client_count() {
	const std::lock_guard lock(_mutex);
	return I(_clients.size());
}

unsigned m2::network::ServerThread::turn_holder() {
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

m2::void_expected m2::network::ServerThread::close_lobby() {
	INFO_FN();
	{
		// Check if all clients reported as ready
		const std::lock_guard lock(_mutex);
		for (auto& client : _clients) {
			if (not client.is_ready()) {
				return make_unexpected("Not every client is ready");
			}
		}
	}

	LOG_INFO("Lobby closed");
	set_state_locked(pb::ServerState::SERVER_READY);
	return {};
}

void m2::network::ServerThread::set_turn_holder_index(unsigned idx) {
	LOG_INFO("New turn holder index", idx);
	const std::lock_guard lock(_mutex);
	_turn_holder = idx;
}

void m2::network::ServerThread::server_update() {
	INFO_FN();
	auto turn = turn_holder();

	auto count = client_count();
	for (auto i = 1; i < count; ++i) { // ServerUpdate is not sent to self
		pb::NetworkMessage message;
		message.set_game_hash(GAME.hash());
		message.set_sender_id(0);
		message.mutable_server_update()->set_receiver_index(i);
		message.mutable_server_update()->set_turn_holder_index(turn);
		for (auto player_id : PROXY.multi_player_object_ids) {
			message.mutable_server_update()->add_player_object_ids(player_id);
		}
		for (auto char_it : LEVEL.characters) { // Iterate over characters
			auto [char_variant, char_id] = char_it;
			auto* object_descriptor = message.mutable_server_update()->add_objects_with_character();

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

		{
			const std::lock_guard lock(_mutex);
			LOG_DEBUG("Queueing outgoing message to client", i);
			_clients[i].push_outgoing_message(std::move(message));
		}
	}
}

void m2::network::ServerThread::set_state_locked(pb::ServerState state) {
	LOG_DEBUG("Setting new state", pb::enum_name(state));
	const std::lock_guard lock(_mutex);
	_state = state;
}

void m2::network::ServerThread::thread_func(ServerThread* server_thread) {
	INFO_FN();

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

	while (not server_thread->is_quit()) {
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
					LOG_INFO("New client connected", server_thread->_clients.size());
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
					LOG_DEBUG("Client socket is readable", i);

					auto fetch_success = client.fetch_incoming_messages(server_thread->_read_buffer, sizeof(server_thread->_read_buffer));
					if (not fetch_success) {
						LOG_ERROR("Failed to fetch message", fetch_success.error());
						server_thread->_clients.clear();
						return;
					}

					// Process message
					if (auto opt_message = client.peak_incoming_message(); opt_message) {
						if (not opt_message->has_client_command() && not opt_message->has_client_update()) {
							LOG_INFO("Received ping", i);
							client.pop_incoming_message(); // Pop the message from the incoming queue
							if (auto sender_id = opt_message->sender_id(); sender_id) {
								if (not client.is_ready()) {
									LOG_INFO("Client ready", i, sender_id);
									client.set_ready(sender_id);
								}
							} else {
								LOG_INFO("Client not ready", i, client.sender_id());
								client.clear_ready();
							}
						} else if (opt_message->has_client_command()) {
							auto json_str = pb::message_to_json_string(*opt_message);
							LOG_DEBUG("Received client command", i, *json_str);
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

	LOG_INFO("Quit");
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
