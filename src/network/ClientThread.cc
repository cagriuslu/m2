#include <m2/network/ClientThread.h>
#include <m2/network/Socket.h>
#include <m2/network/Detail.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/Meta.h>

namespace {
	template <typename NamedItemListT, typename ResourceListT>
	void update_character(m2::Character* c, const NamedItemListT& named_items, const ResourceListT& resources) {
		// Update items
		c->clear_items();
		for (auto named_item_type : named_items) {
			c->add_named_item_no_benefits(M2_GAME.get_named_item(static_cast<m2g::pb::ItemType>(named_item_type)));
		}
		// Update resources
		c->clear_resources();
		for (const auto& resource : resources) {
			c->add_resource(resource.type(), m2::get_resource_amount(resource));
		}
	};
}

m2::network::ClientThread::ClientThread(mplayer::Type type, std::string addr) : _type(type), _addr(std::move(addr)),
		_thread(ClientThread::thread_func, this) {
	INFO_FN();
}

m2::network::ClientThread::~ClientThread() {
	DEBUG_FN();
	set_state_locked(pb::CLIENT_QUIT);
	_thread.join();

	// Give some time for the client thread to initiate TCP shutdown
	m2::sdl::delay(250);
}

bool m2::network::ClientThread::is_not_connected() {
	const std::lock_guard lock(_mutex);
	return _state == pb::CLIENT_NOT_READY;
}

bool m2::network::ClientThread::is_connected() {
	const std::lock_guard lock(_mutex);
	return _state == pb::CLIENT_CONNECTED;
}

bool m2::network::ClientThread::is_ready() {
	const std::lock_guard lock(_mutex);
	return _state == pb::CLIENT_READY;
}

bool m2::network::ClientThread::is_started() {
	const std::lock_guard lock(_mutex);
	return _state == pb::CLIENT_STARTED;
}

std::optional<m2::pb::ServerUpdate> m2::network::ClientThread::peek_unprocessed_server_update() {
	const std::lock_guard lock(_mutex);
	if (_unprocessed_server_update) {
		return _unprocessed_server_update->server_update();
	} else {
		return std::nullopt;
	}
}

std::optional<m2::pb::ServerUpdate> m2::network::ClientThread::last_processed_server_update() {
	const std::lock_guard lock(_mutex);
	if (_last_processed_server_update) {
		return _last_processed_server_update->server_update();
	} else {
		return std::nullopt;
	}
}

bool m2::network::ClientThread::is_our_turn() {
	auto server_update = last_processed_server_update();
	if (server_update) {
		return server_update->turn_holder_index() == server_update->receiver_index();
	} else {
		// Check if we're the host, because the host doesn't receive server updates.
		if (M2_GAME.is_server()) {
			return M2_GAME.server_thread().turn_holder() == 0;
		}
		return false;
	}
}

int m2::network::ClientThread::total_player_count() {
	if (auto server_update = last_processed_server_update(); server_update) {
		return server_update->player_object_ids_size();
	}
	if (auto server_update = peek_unprocessed_server_update(); server_update) {
		return server_update->player_object_ids_size();
	}
	return 0;
}

int m2::network::ClientThread::receiver_index() {
	if (auto server_update = last_processed_server_update(); server_update) {
		return server_update->receiver_index();
	}
	if (auto server_update = peek_unprocessed_server_update(); server_update) {
		return server_update->receiver_index();
	}
	return 0; // Else, the ClientThread must be the server's client.
}

void m2::network::ClientThread::set_ready_blocking(bool state) {
	INFO_FN();

	{
		// Send ping with/without sender_id
		const std::lock_guard lock(_mutex);
		pb::NetworkMessage msg;
		msg.set_game_hash(M2_GAME.hash());
		msg.set_ready(state);
		_message_queue.emplace_back(std::move(msg));
	}

	while (message_count_locked()) {
		SDL_Delay(100); // Wait until output queue is empty
	}

	set_state_locked(state ? pb::CLIENT_READY : pb::CLIENT_CONNECTED);
}

m2::void_expected m2::network::ClientThread::process_server_update() {
	TRACE_FN();

	const std::lock_guard lock(_mutex);

	// Shift the ServerUpdate
	if (not _unprocessed_server_update) {
		// No ServerUpdate to process
		return {};
	}

	LOG_DEBUG("Shifting server update: prev << last << unprocessed << null");
	if (_last_processed_server_update) {
		_prev_processed_server_update = std::move(_last_processed_server_update);
	}
	_last_processed_server_update = std::move(_unprocessed_server_update);
	_unprocessed_server_update.reset();

	if (not _prev_processed_server_update) {
		LOG_DEBUG("Processing first ServerUpdate");
		// This will be the first ServerUpdate, that started the game.
		// Only do verification as level initialization should have initialized the same exact game state
		const auto& server_update = _last_processed_server_update->server_update();

		if (M2G_PROXY.multi_player_object_ids.size() != Z(server_update.player_object_ids_size())) {
			return make_unexpected("Server and local player count doesn't match");
		}

		if (M2_LEVEL.player_id != M2G_PROXY.multi_player_object_ids[server_update.receiver_index()]) {
			return make_unexpected("Player ID doesn't match the ID found in local player list");
		}

		if (M2_LEVEL.characters.size() != Z(server_update.objects_with_character_size())) {
			return make_unexpected("Server and local have different number of characters");
		}

		int i = 0;
		for (auto char_it = M2_LEVEL.characters.begin(); char_it != M2_LEVEL.characters.end() && i < server_update.objects_with_character_size(); ++char_it, ++i) {
			auto [local_character_variant, _] = *char_it;
			auto server_character = server_update.objects_with_character(i);

			auto success = std::visit(overloaded {
					[this, &server_character](const auto& v) -> m2::void_expected {
						if (v.parent().position != VecF{server_character.position()}) {
							return make_unexpected("Server and local position mismatch");
						}
						if (v.parent().object_type() != server_character.object_type()) {
							return make_unexpected("Server and local object type mismatch");
						}
						if (std::distance(v.begin_items(), v.end_items()) != server_character.named_items_size()) {
							return make_unexpected("Server and local item count mismatch");
						}
						// TODO other checks

						// Map server ObjectIDs to local ObjectIDs
						_server_to_local_map[server_character.object_id()] = std::make_pair(v.object_id, true);

						return {};
					}
			}, *local_character_variant);
			m2_reflect_failure(success);
		}

		// Check if server_to_local_map contains all the players in the game
		for (auto player_object_id : server_update.player_object_ids()) {
			if (auto it = _server_to_local_map.find(player_object_id); it == _server_to_local_map.end()) {
				return make_unexpected("Server to local object ID map does not contain a player's ID");
			}
		}

		return {}; // Successfully processed the first ServerUpdate
	}

	LOG_DEBUG("Processing ServerUpdate");
	const auto& server_update = _last_processed_server_update->server_update();
	{
		const auto& prev_server_update = _prev_processed_server_update->server_update();
		// Check that the player IDs haven't changed
		if (prev_server_update.player_object_ids_size() != server_update.player_object_ids_size()) {
			return make_unexpected("Number of players have changed");
		}
		for (int i = 0; i < prev_server_update.player_object_ids_size(); ++i) {
			if (prev_server_update.player_object_ids(i) != server_update.player_object_ids(i)) {
				return make_unexpected("A player's ID has changed");
			}
		}
	}

	// Mark local objects as unvisited
	std::ranges::for_each(_server_to_local_map, [](auto& kv_pair) { kv_pair.second.second = false; });

	struct ObjectToCreate {
		ObjectId server_object_id;
		pb::VecF position;
		m2g::pb::ObjectType object_type;
		ObjectId server_object_parent_id;
		std::vector<int> named_items;
		std::vector<m2::pb::Resource> resources;
	};
	std::vector<ObjectToCreate> objects_to_be_created;

	// Iterate over ServerUpdate objects w/ character
	for (const auto& object_desc : server_update.objects_with_character()) {
		if (auto it = _server_to_local_map.find(object_desc.object_id()); it != _server_to_local_map.end()) {
			LOG_TRACE("Server object is still alive", object_desc.object_id(), it->first);
			// Update the character
			auto* character = M2_LEVEL.objects.get(it->second.first)->get_character();
			update_character(character, object_desc.named_items(), object_desc.resources());
			// Mark object as visited
			it->second.second = true;
		} else {
			// Add details about the object that'll be created into a list
			objects_to_be_created.push_back({object_desc.object_id(), object_desc.position(), object_desc.object_type(),
				object_desc.parent_id(), {object_desc.named_items().begin(), object_desc.named_items().end()},
				{object_desc.resources().begin(), object_desc.resources().end()}});
		}
	}

	// Create new objects. This is more complicated than updating existing objects because of the parent relationships.
	// - Objects without a parent can be created directly.
	// - Objects with a parent which is already created can also be created directly, referring to the local object as the parent.
	// - Objects with a parent which is not yet created cannot be created directly. We need to iterate the objects_to_be_created
	//   list over and over again until all parents are created.
	while (not objects_to_be_created.empty()) {
		// Loop until all objects are created

		// Iterate over objects to be created
		auto it = objects_to_be_created.begin();
		while (it != objects_to_be_created.end()) {
			// If the new object has no parent
			if (it->server_object_parent_id == 0) {
				// Simply, create the object
				LOG_DEBUG("Server has created an object", it->server_object_id);
				auto [obj, id] = m2::create_object(m2::VecF{it->position}, it->object_type, 0);
				auto load_result = M2G_PROXY.init_fg_object(obj);
				m2_reflect_failure(load_result);
				// Update the character
				auto* character = obj.get_character();
				update_character(character, it->named_items, it->resources);
				// Add object to the map, marked as visited
				_server_to_local_map[it->server_object_id] = std::make_pair(id, true);
				// Delete the object details from the objects_to_be_created vector
				it = objects_to_be_created.erase(it);
			} else if (auto parent_it = _server_to_local_map.find(it->server_object_parent_id); parent_it != _server_to_local_map.end()) {
				// If the object has a parent that's already created, create the object by looking up the corresponding parent
				LOG_DEBUG("Server has created an object", it->server_object_id);
				auto [obj, id] = m2::create_object(m2::VecF{it->position}, it->object_type, parent_it->second.first);
				auto load_result = M2G_PROXY.init_fg_object(obj);
				m2_reflect_failure(load_result);
				// Update the character
				auto* character = obj.get_character();
				update_character(character, it->named_items, it->resources);
				// Add object to the map, marked as visited
				_server_to_local_map[it->server_object_id] = std::make_pair(id, true);
				// Delete the object details from the objects_to_be_created vector
				it = objects_to_be_created.erase(it);
			} else {
				LOG_TRACE("Server has created an object with a parent, but the parent is not created locally yet", it->server_object_id, it->server_object_parent_id);
				++it;
			}
		}
	}

	// Iterate over map, delete the objects that haven't been visited, as they must have been deleted on the server side
	for (auto it = _server_to_local_map.cbegin(); it != _server_to_local_map.cend(); /* no increment */) {
		if (!it->second.second) {
			auto object_to_delete = it->second.first;
			LOG_DEBUG("Local object hasn't been visited by ServerUpdate, scheduling for deletion", it->second.first);
			M2_DEFER(create_object_deleter(object_to_delete));
			it = _server_to_local_map.erase(it); // Erase from map
		} else {
			++it;
		}
	}

	return {};
}

void m2::network::ClientThread::queue_client_command(const m2g::pb::ClientCommand& cmd) {
	INFO_FN();

	pb::NetworkMessage msg;
	msg.set_game_hash(M2_GAME.hash());
	msg.mutable_client_command()->CopyFrom(cmd);

	const std::lock_guard lock(_mutex);
	_message_queue.emplace_back(std::move(msg));
}

size_t m2::network::ClientThread::message_count_locked() {
	const std::lock_guard lock(_mutex);
	return _message_queue.size();
}

void m2::network::ClientThread::set_state_unlocked(pb::ClientState state) {
	LOG_DEBUG("Setting new state", pb::enum_name(state));
	_state = state;
}

void m2::network::ClientThread::set_state_locked(pb::ClientState state) {
	const std::lock_guard lock(_mutex);
	set_state_unlocked(state);
}

void m2::network::ClientThread::thread_func(ClientThread* client_thread) {
	INFO_FN();

	auto pop_message = [client_thread]() -> std::optional<pb::NetworkMessage> {
		const std::lock_guard lock(client_thread->_mutex);
		if (!client_thread->_message_queue.empty()) {
			LOG_DEBUG("Popping message from outgoing queue");
			auto tmp = std::move(client_thread->_message_queue.front());
			client_thread->_message_queue.pop_front();
			return std::move(tmp);
		} else {
			return std::nullopt;
		}
	};
	auto send_message_locked = [client_thread](Socket& socket, const pb::NetworkMessage& msg) {
		const std::lock_guard lock(client_thread->_mutex);
		if (auto expect_json_str = pb::message_to_json_string(msg); expect_json_str) {
			LOG_DEBUG("Will send message", *expect_json_str);
			auto send_success = socket.send(expect_json_str->data(), expect_json_str->size());
			if (not send_success) {
				LOG_ERROR("Send error", send_success.error());
			}
		}
	};

	std::optional<Socket> socket;
	while (not client_thread->is_quit()) {
		if (client_thread->is_not_connected()) {
			auto expect_socket = Socket::create_tcp();
			if (not expect_socket) {
				LOG_FATAL("Socket creation failed", expect_socket.error());
				return;
			}
			socket = std::move(*expect_socket);
			LOG_INFO("Socket created");

			auto connect_success = socket->connect(client_thread->_addr, 1162);
			if (not connect_success) {
				LOG_FATAL("Connect failed", connect_success.error());
				return;
			}
			LOG_INFO("Connected");
			client_thread->set_state_locked(pb::CLIENT_CONNECTED);
		} else {
			fd_set read_set;
			FD_ZERO(&read_set);
			FD_SET(socket->fd(), &read_set);

			auto select_result = select(socket->fd(), &read_set, nullptr, 100);
			if (not select_result) {
				LOG_FATAL("Select failed", select_result.error());
				return;
			}

			if (0 < *select_result) {
				if (not FD_ISSET(socket->fd(), &read_set)) {
					// Skip if no messages have been received
					continue;
				}

				auto recv_success = socket->recv(client_thread->_read_buffer, sizeof(client_thread->_read_buffer));
				if (not recv_success) {
					LOG_ERROR("Receive failed", recv_success.error());
					continue;
				}
				if (*recv_success == 0) {
					LOG_ERROR("Server disconnected");
					socket.reset();

					const std::lock_guard lock(client_thread->_mutex);
					client_thread->set_state_unlocked(pb::CLIENT_NOT_READY);

					continue;
				}

				// Parse message
				std::string json_str{client_thread->_read_buffer, static_cast<size_t>(*recv_success)};
				auto expect_message = pb::json_string_to_message<pb::NetworkMessage>(json_str);
				if (not expect_message) {
					LOG_ERROR("Received bad message", expect_message.error(), json_str);
					continue;
				}

				if (expect_message->game_hash() != M2_GAME.hash()) {
					LOG_ERROR("Received message of unknown origin", json_str);
					continue;
				}

				if (not expect_message->has_server_command() && not expect_message->has_server_update()) {
					LOG_INFO("Received ping");
				} else if (expect_message->has_server_update()) {
					LOG_INFO("Received ServerUpdate", json_str);
					// Wait until the previous ServerUpdate is processed
					while (client_thread->_unprocessed_server_update) {
						SDL_Delay(100); // Yield the thread while waiting
					}

					{
						const std::lock_guard lock(client_thread->_mutex);
						client_thread->_unprocessed_server_update = std::move(*expect_message);
						if (client_thread->_state != pb::CLIENT_STARTED) {
							// Set the state as STARTED when the first ServerUpdate is received.
							client_thread->set_state_unlocked(pb::CLIENT_STARTED);
						}
					}
				} else {
					// TODO process other incoming messages
				}
			}

			// Write outgoing messages
			size_t msg_count = client_thread->message_count_locked();
			for (size_t i = 0; i < msg_count; ++i) {
				auto expect_msg = pop_message();
				if (not expect_msg) {
					break; // No message left in queue
				}
				send_message_locked(*socket, *expect_msg);
			}
		}
	}
}

bool m2::network::ClientThread::is_quit() {
	const std::lock_guard lock(_mutex);
	return _state == pb::ClientState::CLIENT_QUIT;
}
