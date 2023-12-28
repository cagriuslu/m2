#include <m2/network/ClientThread.h>
#include <m2/network/Socket.h>
#include <m2/network/Detail.h>
#include <m2/network/FdSet.h>
#include <m2/Game.h>
#include <m2/ProxyDetail.h>
#include <m2/Log.h>
#include <m2/Meta.h>

m2::network::ClientThread::ClientThread(mplayer::Type type, std::string addr) : _type(type), _addr(std::move(addr)),
		_thread(ClientThread::thread_func, this) {}

m2::network::ClientThread::~ClientThread() {
	set_state_locked(State::Quit);
	_thread.join();
}

bool m2::network::ClientThread::is_connected() {
	const std::lock_guard lock(_mutex);
	return _state == State::Connected;
}

bool m2::network::ClientThread::is_ready() {
	const std::lock_guard lock(_mutex);
	return _state == State::Ready;
}

bool m2::network::ClientThread::is_started() {
	const std::lock_guard lock(_mutex);
	return _state == State::Started;
}

void m2::network::ClientThread::set_ready_blocking(bool state) {
	queue_ping_locked(state ? GAME.sender_id() : 0);
	while (message_count_locked()) {
		SDL_Delay(100); // Wait until output queue is empty
	}
	set_state_locked(state ? State::Ready : State::Connected);
}

size_t m2::network::ClientThread::message_count_locked() {
	const std::lock_guard lock(_mutex);
	return _message_queue.size();
}

void m2::network::ClientThread::set_state_locked(State state) {
	const std::lock_guard lock(_mutex);
	_state = state;
}

void m2::network::ClientThread::queue_ping_locked(int32_t sender_id) {
	const std::lock_guard lock(_mutex);
	pb::NetworkMessage msg;
	msg.set_game_hash(game_hash());
	msg.set_sender_id(sender_id);
	_message_queue.emplace_back(std::move(msg));
}

void m2::network::ClientThread::thread_func(ClientThread* client_thread) {
	auto is_quit = [client_thread]() {
		const std::lock_guard lock(client_thread->_mutex);
		return client_thread->_state == State::Quit;
	};
	auto pop_message = [client_thread]() -> std::optional<pb::NetworkMessage> {
		const std::lock_guard lock(client_thread->_mutex);
		if (!client_thread->_message_queue.empty()) {
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
			auto send_success = socket.send(expect_json_str->data(), expect_json_str->size());
			if (not send_success) {
				LOG_ERROR("Send error", send_success.error());
			}
		}
	};

	auto socket = Socket::create_tcp();
	if (not socket) {
		LOG_FATAL("Socket creation failed", socket.error());
		return;
	}

	auto connect_success = socket->connect(client_thread->_addr, 1162);
	if (not connect_success) {
		LOG_FATAL("Connect failed", connect_success.error());
		return;
	}
	client_thread->set_state_locked(State::Connected);
	LOG_INFO("Connected");

	FdSet read_set, write_set;
	read_set.add_fd(socket->fd());

	while (!is_quit()) {
		auto select_result = select(read_set, write_set, 100);
		if (not select_result) {
			LOG_FATAL("Select failed", select_result.error());
			return;
		}

		if (*select_result == 0) {
			// Timeout
			LOG_TRACE("Select timeout");
		} else {
			// TODO check sockets
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
