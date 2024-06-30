#include <m2/network/server/Client.h>
#include <m2/protobuf/Detail.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <string>

m2::expected<bool> m2::network::server::Client::save_incoming_message(char* read_buffer, size_t read_buffer_length) {
	if (_incoming_msg && _buffered_incoming_msg) {
		LOG_WARN("Found unread messages in inbox, skipping reading new ones");
		return false; // This is not an important error, we may try again
	}

	auto recv_success = _socket->recv(read_buffer, read_buffer_length);
	m2_reflect_failure(recv_success);

	if (*recv_success == 0) {
		LOG_WARN("Client closed socket, closing our side as well");
		_socket.reset();
		return false; // This is not an important error
	}

	// Parse the first message up to the null character
	std::string first_json_str{read_buffer};
	auto expect_first_message = pb::json_string_to_message<pb::NetworkMessage>(first_json_str);
	m2_reflect_failure(expect_first_message);
	// Check game_hash
	if (expect_first_message->game_hash() != M2_GAME.hash()) {
		return make_unexpected("Client game hash mismatch");
	}
	if (not _incoming_msg.has_value() && not _buffered_incoming_msg.has_value()) {
		_incoming_msg.emplace(std::move(*expect_first_message));
	} else if (_incoming_msg.has_value() && not _buffered_incoming_msg.has_value()) {
		_buffered_incoming_msg.emplace(std::move(*expect_first_message));
	} else {
		throw M2FATAL("Invalid buffer state");
	}
	LOG_DEBUG("Saved incoming client message");

	// Parse the second message if exists
	if (first_json_str.size() + 1 < static_cast<size_t>(*recv_success)) {
		// A second message have been received
		if (_buffered_incoming_msg) {
			return m2::make_unexpected("Out of buffer space");
		}

		std::string second_json_str{read_buffer + first_json_str.size() + 1};
		auto expect_second_message = pb::json_string_to_message<pb::NetworkMessage>(second_json_str);
		m2_reflect_failure(expect_second_message);
		// Check game_hash
		if (expect_second_message->game_hash() != M2_GAME.hash()) {
			return make_unexpected("Client game hash mismatch");
		}
		_buffered_incoming_msg.emplace(std::move(*expect_second_message));
		LOG_DEBUG("Saved second incoming client message");
	}

	return true;
}

const std::optional<m2::pb::NetworkMessage>& m2::network::server::Client::peak_incoming_message() {
	return _incoming_msg;
}

std::optional<m2::pb::NetworkMessage> m2::network::server::Client::pop_incoming_message() {
	if (_incoming_msg) {
		LOG_DEBUG("Popping client message");
		auto msg = std::move(*_incoming_msg);
		_incoming_msg.reset();

		if (_buffered_incoming_msg) {
			_incoming_msg = std::move(_buffered_incoming_msg);
			_buffered_incoming_msg.reset();
		}

		return std::move(msg);
	}
	return std::nullopt;
}

void m2::network::server::Client::push_outgoing_message(pb::NetworkMessage msg) {
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

		// Send the null character at the end of the message as a separator between messages
		auto send_success = _socket->send(expect_string->c_str(), expect_string->size() + 1);
		m2_reflect_failure(send_success);
	}
	return (0 < msg_count);
}
