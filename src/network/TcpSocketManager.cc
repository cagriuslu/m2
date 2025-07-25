#include <m2/network/TcpSocketManager.h>
#include <m2/protobuf/Detail.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <string>
#include <m2/sdl/Detail.h>

m2::expected<m2::network::TcpSocketManager::ReadResult> m2::network::TcpSocketManager::read_incoming_data(std::queue<pb::TurnBasedNetworkMessage>& read_to) {
	if (_incoming_buffer.size() <= _incoming_buffer_next_available_position) {
		return TcpSocketManager::ReadResult::BUFFER_OVERFLOW;
	}

	// Assuming that the socket is already readable, read from the next available position at the buffer
	auto received_byte_count = _socket.recv(_incoming_buffer.data() + _incoming_buffer_next_available_position,
		_incoming_buffer.size() - _incoming_buffer_next_available_position);
	m2ReflectUnexpected(received_byte_count);

	if (*received_byte_count == 0) {
		// Assuming the socket is blocking and no receive timeout is set
		throw M2_ERROR("EAGAIN occurred on a blocking socket with no receive timeout");
	}
	LOG_DEBUG("Received data from peer", _index, *received_byte_count);

	// Advance the pointer to the next available position
	_incoming_buffer_next_available_position += *received_byte_count;

	// Check if there are messages in the buffer
	size_t next_possible_message_position = 0, vacuum_count = 0;
	while (next_possible_message_position < _incoming_buffer_next_available_position) {
		// Search the buffer for a nil character, which signifies the end of a message, starting from next_possible_message_position
		size_t strlen = 0;
		bool found_whole_message = false;
		for (size_t i = next_possible_message_position; i < _incoming_buffer_next_available_position; ++i) {
			if (_incoming_buffer[i]) {
				++strlen;
			} else {
				found_whole_message = true;
				break;
			}
		}

		if (not found_whole_message) {
			// Message is not yet complete
			break;
		}

		// Found a message that starts at `next_possible_message_position` and is `strlen` length long
		std::string possible_message{_incoming_buffer.data() + next_possible_message_position, strlen};
		// Parse as protobuf
		auto message = pb::json_string_to_message<pb::TurnBasedNetworkMessage>(possible_message);
		if (not message) {
			LOG_ERROR("Invalid message received from peer", _index, message.error());
			return TcpSocketManager::ReadResult::INVALID_MESSAGE;
		}
		// Check game_hash
		if (message->game_hash() != M2_GAME.Hash()) {
			LOG_ERROR("Game hash mismatch", _index, possible_message);
			return TcpSocketManager::ReadResult::GAME_HASH_MISMATCH;
		}
		LOG_INFO("Saving message to incoming queue of self", _index, possible_message);
		read_to.emplace(std::move(*message));
		// Increase vacuum count so that the buffer is compacted
		vacuum_count += strlen + 1; // Don't forget the nil character
		// Keep looking for other messages
		next_possible_message_position += strlen + 1;
	}

	// Vacuum the buffer
	if (vacuum_count) {
		memcpy(_incoming_buffer.data(), _incoming_buffer.data() + vacuum_count, _incoming_buffer_next_available_position - vacuum_count);
		_incoming_buffer_next_available_position -= vacuum_count;
	}

	return read_to.empty() ? TcpSocketManager::ReadResult::INCOMPLETE_MESSAGE_RECEIVED : TcpSocketManager::ReadResult::MESSAGE_RECEIVED;
}

m2::expected<m2::network::TcpSocketManager::SendResult> m2::network::TcpSocketManager::send_outgoing_data(std::queue<pb::TurnBasedNetworkMessage>& read_from) {
	// We don't want to block the server thread indefinitely. So either send what's inside _outgoing_buffer, or try to
	// send one message from the _outgoing_queue.

	if (_outgoing_buffer_bytes_left == 0) {
		if (read_from.empty()) {
			// Nothing to do
			return SendResult::OK;
		}
		// Extract message from outgoing queue
		auto msg = std::move(read_from.front());
		read_from.pop();
		// Convert to string
		auto str = pb::message_to_json_string(msg);
		if (not str) {
			LOG_ERROR("Invalid message in outgoing queue");
			return SendResult::INVALID_MESSAGE;
		}
		if (_outgoing_buffer.size() < str->length() + 1) {
			LOG_ERROR("Message would overflow the outgoing buffer", *str);
			return SendResult::BUFFER_WOULD_OVERFLOW;
		}
		LOG_INFO("Moving message from outgoing queue of self to outgoing buffer", _index, *str);
		memcpy(_outgoing_buffer.data(), str->c_str(), str->length() + 1); // Don't forget the nil character
		_outgoing_buffer_start_position = 0;
		_outgoing_buffer_bytes_left = str->length() + 1;
	}

	// Assuming the socket is in writable state, try to send everything in _outgoing_buffer
	auto sent_byte_count = _socket.send(_outgoing_buffer.data() + _outgoing_buffer_start_position,
		_outgoing_buffer_bytes_left);
	m2ReflectUnexpected(sent_byte_count);
	if (*sent_byte_count == -1 || *sent_byte_count == 0) {
		// EAGAIN occurred or no bytes were sent, try again later
		return SendResult::OK;
	}
	LOG_DEBUG("Sent data to peer", _index, *sent_byte_count);
	// Move position pointers further
	_outgoing_buffer_start_position += *sent_byte_count;
	_outgoing_buffer_bytes_left -= *sent_byte_count;
	return SendResult::OK;
}

void m2::network::TcpSocketManager::flush(std::queue<pb::TurnBasedNetworkMessage>& read_from, int timeout_ms) {
	auto has_anything_to_send = [&](){
		return (has_outgoing_data() || not read_from.empty());
	};

	// Early out
	if (not has_anything_to_send()) {
		return;
	}

	auto start_ticks = sdl::get_ticks();
	while ((sdl::get_ticks() - start_ticks < timeout_ms) && has_anything_to_send()) {
		if (auto send_result = send_outgoing_data(read_from); not send_result || send_result != SendResult::OK) {
			LOG_WARN("Error while flushing", send_result.error());
			break;
		}
	}
	if (read_from.empty()) {
		LOG_DEBUG("Outgoing data has drained");
	} else {
		LOG_DEBUG("Timeout occurred while flushing");
	}
}
