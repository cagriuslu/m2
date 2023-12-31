#include <m2/network/ClientThread.h>
#include <m2/network/Socket.h>
#include <m2/network/Detail.h>
#include <m2/network/FdSet.h>
#include <m2/Game.h>
#include <m2/ProxyDetail.h>
#include <m2/Log.h>
#include <m2/Meta.h>

m2::network::ClientThread::ClientThread(mplayer::Type type, std::string addr) : _type(type), _addr(std::move(addr)),
		_thread(ClientThread::thread_func, this) {
	DEBUG_FN();
}

m2::network::ClientThread::~ClientThread() {
	DEBUG_FN();
	set_state_locked(pb::CLIENT_QUIT);
	_thread.join();
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
		// Check if we're the host
		if (GAME.is_server()) {
			return GAME.server_thread().turn_holder_index() == 0;
		}
		return false;
	}
}

void m2::network::ClientThread::set_ready_blocking(bool state) {
	DEBUG_FN();
	queue_ping_locked(state ? GAME.sender_id() : 0);
	while (message_count_locked()) {
		SDL_Delay(100); // Wait until output queue is empty
	}
	set_state_locked(state ? pb::CLIENT_READY : pb::CLIENT_CONNECTED);
}

m2::expected<bool> m2::network::ClientThread::process_server_update() {
	TRACE_FN();
	const std::lock_guard lock(_mutex);
	if (not fetch_server_update_unlocked()) {
		// No ServerUpdate to process
		return false;
	}

	if (not _prev_processed_server_update) {
		LOG_DEBUG("Processing first ServerUpdate");
		// This will be the first ServerUpdate, that started the game.
		// Only do verification as level initialization should have initialized the same exact game state
		const auto& server_update = _last_processed_server_update->server_update();

		if (m2g::multi_player_object_ids.size() != Z(server_update.player_object_ids_size())) {
			return make_unexpected("Server and local player count doesn't match");
		}

		if (LEVEL.player_id != m2g::multi_player_object_ids[server_update.receiver_index()]) {
			return make_unexpected("Player ID doesn't match the ID found in local player list");
		}

		if (LEVEL.characters.size() != Z(server_update.objects_with_character_size())) {
			return make_unexpected("Server and local have different number of characters");
		}

		int i = 0;
		for (auto char_it = LEVEL.characters.begin(); char_it != LEVEL.characters.end() && i < server_update.objects_with_character_size(); ++char_it, ++i) {
			auto [local_character_variant, _] = *char_it;
			auto server_character = server_update.objects_with_character(i);

			auto success = std::visit(overloaded {
					[&server_character](const auto& v) -> m2::void_expected {
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
						return {};
					}
			}, *local_character_variant);
			m2_reflect_failure(success);
		}

		return true; // Successfully processed one ServerUpdate
	}

	const auto& server_update = _last_processed_server_update->server_update();
	const auto& prev_server_update = _prev_processed_server_update->server_update();
	// TODO actual ServerUpdate

	return {};
}

void m2::network::ClientThread::queue_client_command(const m2g::pb::ClientCommand& cmd) {
	DEBUG_FN();
	pb::NetworkMessage msg;
	msg.set_game_hash(game_hash());
	msg.set_sender_id(GAME.sender_id());
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

void m2::network::ClientThread::queue_ping_locked(int32_t sender_id) {
	DEBUG_FN();
	const std::lock_guard lock(_mutex);
	pb::NetworkMessage msg;
	msg.set_game_hash(game_hash());
	msg.set_sender_id(sender_id);
	_message_queue.emplace_back(std::move(msg));
}

bool m2::network::ClientThread::fetch_server_update_unlocked() {
	if (_unprocessed_server_update) {
		LOG_DEBUG("Fetching server update");
		if (_last_processed_server_update) {
			_prev_processed_server_update = std::move(_last_processed_server_update);
		}
		_last_processed_server_update = std::move(_unprocessed_server_update);
		_unprocessed_server_update.reset();
		return true;
	}
	return false;
}

void m2::network::ClientThread::thread_func(ClientThread* client_thread) {
	DEBUG_FN();
	auto is_quit = [client_thread]() {
		const std::lock_guard lock(client_thread->_mutex);
		return client_thread->_state == pb::CLIENT_QUIT;
	};
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
	FdSet read_set, write_set;

	while (!is_quit()) {
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
			LOG_INFO("Socket connected");
			client_thread->set_state_locked(pb::CLIENT_CONNECTED);
			read_set.add_fd(socket->fd());
		} else {
			auto select_result = select(read_set, write_set, 100);
			if (not select_result) {
				LOG_FATAL("Select failed", select_result.error());
				return;
			}

			if (0 < *select_result) {
				const std::lock_guard lock(client_thread->_mutex);
				if (not read_set.is_set(socket->fd())) {
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
					read_set.remove_fd(socket->fd());
					socket.reset();
					client_thread->set_state_unlocked(pb::CLIENT_NOT_READY);
					continue;
				}

				// Parse message
				std::string json_str{client_thread->_read_buffer, static_cast<size_t>(*recv_success)};
				auto expect_message = pb::json_string_to_message<pb::NetworkMessage>(json_str);
				if (not expect_message) {
					LOG_ERROR("Received bad message", json_str);
					continue;
				}

				if (expect_message->game_hash() != game_hash()) {
					LOG_ERROR("Received message of unknown origin", json_str);
					continue;
				}

				if (not expect_message->has_server_command() && not expect_message->has_server_update()) {
					LOG_INFO("Received ping");
				} else if (expect_message->has_server_update()) {
					LOG_INFO("Received ServerUpdate", json_str);
					client_thread->_unprocessed_server_update = std::move(*expect_message);
					client_thread->set_state_unlocked(pb::CLIENT_STARTED);
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
