#include <m2/network/server/Client.h>
#include <m2/protobuf/Detail.h>
#include <m2/Game.h>
#include <string>

m2::void_expected m2::network::server::Client::fetch_incoming_messages(char* read_buffer, size_t read_buffer_length) {
	auto recv_success = _socket->recv(read_buffer, read_buffer_length);
	m2_reflect_failure(recv_success);

	if (*recv_success == 0) {
		_socket.reset(); // Close socket from our side as well
		return {}; // This is not an important error, the client may connect again
	}

	// Parse message
	std::string json_str{read_buffer, static_cast<size_t>(*recv_success)};
	auto expect_message = pb::json_string_to_message<pb::NetworkMessage>(json_str);
	m2_reflect_failure(recv_success);

	// Check game_hash
	if (expect_message->game_hash() != GAME.hash()) {
		return make_unexpected("Client game hash mismatch");
	}

	_incoming_queue.push(std::move(*expect_message));
	return {};
}

m2::expected<bool> m2::network::server::Client::flush_outgoing_messages() {
	auto msg_count = _outgoing_queue.size(); // Read message count separate to prevent deadloops
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

std::optional<m2::pb::NetworkMessage> m2::network::server::Client::peak_incoming_message() {
	if (not _incoming_queue.empty()) {
		return _incoming_queue.front();
	}
	return std::nullopt;
}

std::optional<m2::pb::NetworkMessage> m2::network::server::Client::pop_incoming_message() {
	if (not _incoming_queue.empty()) {
		auto msg = std::move(_incoming_queue.front());
		_incoming_queue.pop();
		return std::move(msg);
	}
	return std::nullopt;
}

void m2::network::server::Client::push_outgoing_message(pb::NetworkMessage&& msg) {
	_outgoing_queue.emplace(std::move(msg));
}
