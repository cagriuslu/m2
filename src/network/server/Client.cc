#include <m2/network/server/Client.h>
#include <m2/protobuf/Detail.h>
#include <m2/Game.h>
#include <string>

m2::expected<std::optional<m2::pb::NetworkMessage>> m2::network::server::Client::save_incoming_message(char* read_buffer, size_t read_buffer_length) {
	DEBUG_FN();

	if (_incoming_msg) {
		LOG_WARN("There's an unprocessed incoming message");
		return std::nullopt; // This is not an important error, we may try again
	}

	auto recv_success = _socket->recv(read_buffer, read_buffer_length);
	m2_reflect_failure(recv_success);

	if (*recv_success == 0) {
		LOG_WARN("Client socket closed, closing our side of the socket");
		_socket.reset();
		return std::nullopt; // This is not an important error
	}

	// Parse message
	std::string json_str{read_buffer, static_cast<size_t>(*recv_success)};
	auto expect_message = pb::json_string_to_message<pb::NetworkMessage>(json_str);
	m2_reflect_failure(recv_success);

	// Check game_hash
	if (expect_message->game_hash() != M2_GAME.hash()) {
		return make_unexpected("Client game hash mismatch");
	}

	LOG_DEBUG("Saving incoming client message");
	_incoming_msg.emplace(std::move(*expect_message));
	return _incoming_msg;
}

std::optional<m2::pb::NetworkMessage> m2::network::server::Client::peak_incoming_message() {
	return _incoming_msg;
}

std::optional<m2::pb::NetworkMessage> m2::network::server::Client::pop_incoming_message() {
	if (_incoming_msg) {
		auto msg = std::move(*_incoming_msg);
		_incoming_msg.reset();
		return std::move(msg);
	}
	return std::nullopt;
}

void m2::network::server::Client::push_outgoing_message(pb::NetworkMessage&& msg) {
	DEBUG_FN();
	_outgoing_queue.emplace(std::move(msg));
}

m2::expected<bool> m2::network::server::Client::flush_outgoing_messages() {
	TRACE_FN();

	auto msg_count = _outgoing_queue.size(); // Read message count separately to prevent deadlocks
	for (size_t i = 0; i < msg_count; ++i) {
		auto msg = std::move(_outgoing_queue.front());
		_outgoing_queue.pop();

		auto expect_string = pb::message_to_json_string(msg);
		m2_reflect_failure(expect_string);

		auto send_success = _socket->send(expect_string->data(), expect_string->size());
		m2_reflect_failure(send_success);
	}
	return (0 < msg_count);
}
