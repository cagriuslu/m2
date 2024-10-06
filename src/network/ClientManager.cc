#include <m2/network/ClientManager.h>
#include <m2/Log.h>

void m2::network::ClientManager::set_misbehaved() {
	clear_socket_and_reset_queues();
	_misbehaved = true;
}

bool m2::network::ClientManager::has_incoming_data(bool is_socket_readable) {
	if (is_socket_readable) {
		auto read_result = _socket_manager->read_incoming_data(_incoming_queue);
		if (not read_result) {
			LOG_WARN("Error occurred while reading, closing connection to client", _socket_manager->index(), read_result.error());
			clear_socket_and_reset_queues();
			return false;
		}
		if (*read_result != TcpSocketManager::ReadResult::MESSAGE_RECEIVED && *read_result != TcpSocketManager::ReadResult::INCOMPLETE_MESSAGE_RECEIVED) {
			LOG_WARN("Invalid data received from client, closing connection", _socket_manager->index(), static_cast<int>(*read_result));
			clear_socket_and_reset_queues();
			return false;
		}
	}
	return not _incoming_queue.empty();
}
const m2::pb::NetworkMessage* m2::network::ClientManager::peak_incoming_message() {
	if (_incoming_queue.empty()) {
		return nullptr;
	} else {
		return &_incoming_queue.front();
	}
}
std::optional<m2::pb::NetworkMessage> m2::network::ClientManager::pop_incoming_message() {
	if (_incoming_queue.empty()) {
		return std::nullopt;
	}

	auto msg = std::move(_incoming_queue.front());
	auto json_str = pb::message_to_json_string(msg);
	LOG_INFO("Popping message from incoming queue of client", _socket_manager->index(), json_str->c_str());
	_incoming_queue.pop();
	return std::move(msg);
}
bool m2::network::ClientManager::has_outgoing_data() const {
	if (not _socket_manager) {
		return false;
	}
	return _socket_manager->has_outgoing_data() || not _outgoing_queue.empty();
}
void m2::network::ClientManager::queue_outgoing_message(m2::pb::NetworkMessage msg) {
	_outgoing_queue.emplace(std::move(msg));
}
void m2::network::ClientManager::send_outgoing_data() {
	auto send_result = _socket_manager->send_outgoing_data(_outgoing_queue);
	if (not send_result) {
		LOG_WARN("Error occurred while writing, closing connection to client", _socket_manager->index(), send_result.error());
		clear_socket_and_reset_queues();
		return;
	}
	if (*send_result != TcpSocketManager::SendResult::OK) {
		throw M2_ERROR("An invalid or too large outgoing message was queued to client: " + std::to_string(_socket_manager->index()) + " " + std::to_string(static_cast<int>(*send_result)));
	}
}
void m2::network::ClientManager::flush_and_shutdown() {
	if (not _socket_manager) {
		return;
	}
	_socket_manager->flush(_outgoing_queue, 10000);
	clear_socket_and_reset_queues();
}
void m2::network::ClientManager::clear_socket_and_reset_queues() {
	_socket_manager.reset();
	_incoming_queue = {};
	_outgoing_queue = {};
	untrusted = true;
}
