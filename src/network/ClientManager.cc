#include <m2/network/ClientManager.h>
#include <m2/Log.h>

m2::network::ClientManager::ClientManager(TcpSocket&& socket, int index)
	: _index(index), _ip_address_and_port(socket.ip_address_and_port()),
	_state(Connected{TcpSocketManager{std::move(socket), index}}) {}

bool m2::network::ClientManager::is_connected() const {
	return std::holds_alternative<Connected>(_state) || std::holds_alternative<Ready>(_state)
	    || std::holds_alternative<ReconnectedUntrusted>(_state);
}
m2::network::TcpSocket& m2::network::ClientManager::tcp_socket() {
	return std::visit(m2::overloaded{
			[](Connected& s) -> TcpSocket& { return s.socket_manager.socket(); },
			[](Ready& s) -> TcpSocket& { return s.socket_manager.socket(); },
			[](ReconnectedUntrusted& s) -> TcpSocket& { return s.socket_manager.socket(); },
			[](auto&) -> TcpSocket& { throw M2_ERROR("TcpSocket unavailable, client is not connected"); },
		}, _state);
}
bool m2::network::ClientManager::is_ready() const {
	return std::holds_alternative<Ready>(_state);
}
bool m2::network::ClientManager::is_disconnected() const {
	return std::holds_alternative<HonorablyDisconnected>(_state);
}
bool m2::network::ClientManager::is_untrusted() const {
	return std::holds_alternative<ReconnectedUntrusted>(_state);
}
bool m2::network::ClientManager::is_disconnected_or_untrusted() const {
	return std::visit(m2::overloaded{
		[](HonorablyDisconnected&) { return true; },
		[](ReconnectedUntrusted&) { return true; },
		[](auto&) { return false; },
	}, _state);
}
std::optional<m2::sdl::ticks_t> m2::network::ClientManager::disconnected_or_untrusted_since() const {
	return std::visit(m2::overloaded{
		[](HonorablyDisconnected& s) -> std::optional<m2::sdl::ticks_t> { return s.disconnected_at; },
		[](ReconnectedUntrusted& s) -> std::optional<m2::sdl::ticks_t> { return s.reconnected_at; },
		[](auto&) -> std::optional<m2::sdl::ticks_t> { return std::nullopt; },
	}, _state);
}
bool m2::network::ClientManager::has_misbehaved() const {
	return std::holds_alternative<Misbehaved>(_state);
}

bool m2::network::ClientManager::set_ready_token(uint64_t ready_token) {
	if (std::holds_alternative<Connected>(_state)) {
		// Switch to ready state
		auto socket_manager = std::move(std::get<Connected>(_state).socket_manager);
		_state = Ready{ .socket_manager = std::move(socket_manager), .ready_token = ready_token };
		return true;
	} else if (std::holds_alternative<ReconnectedUntrusted>(_state)) {
		// Check if the ready_token matches
		if (std::get<ReconnectedUntrusted>(_state).expected_ready_token == ready_token) {
			// Switch to ready state
			auto socket_manager = std::move(std::get<ReconnectedUntrusted>(_state).socket_manager);
			_state = Ready{ .socket_manager = std::move(socket_manager), .ready_token = ready_token };
			return true;
		} else {
			// Token mismatch
			return false;
		}
	} else {
		throw M2_ERROR("Unexpected ready token");
	}
}
void m2::network::ClientManager::honorably_disconnect() {
	auto ready_token = std::visit(m2::overloaded{
		[](Connected&) -> uint64_t { return 0; },
		[](Ready& s) -> uint64_t { return s.ready_token; },
		[](ReconnectedUntrusted& s) -> uint64_t { return s.expected_ready_token; },
		[](auto&) -> uint64_t { throw M2_ERROR("Unexpected call"); },
	}, _state);
	_state = HonorablyDisconnected{ ready_token, sdl::get_ticks() };
}


void m2::network::ClientManager::untrusted_client_reconnected(TcpSocket&& socket) {
	if (socket.ip_address_and_port() != ip_address_and_port()) {
		throw M2_ERROR("Address and/or port mismatch");
	}
	if (not std::holds_alternative<HonorablyDisconnected>(_state)) {
		auto ready_token = std::get<HonorablyDisconnected>(_state).expected_ready_token;
		_state = ReconnectedUntrusted{ .socket_manager = TcpSocketManager{std::move(socket), _index}, .expected_ready_token = ready_token, .reconnected_at = sdl::get_ticks() };
	}
	throw M2_ERROR("Unexpected new socket");
}
void m2::network::ClientManager::set_misbehaved() {
	_state = Misbehaved{};
}

bool m2::network::ClientManager::has_incoming_data(bool is_socket_readable) {
	if (is_socket_readable) {
		auto& socket_manager_ = socket_manager();
		auto& incoming_queue_ = incoming_queue();
		auto read_result = socket_manager_.read_incoming_data(incoming_queue_);
		if (not read_result) {
			// If the client TCP connection is broken, select returns that the socket is readable, and we fail here.
			// So this warning might also mean that the connection just timed out.
			LOG_WARN("Error occurred while reading, closing connection to client", socket_manager_.index(), read_result.error());
			honorably_disconnect();
			return false;
		}
		if (*read_result != TcpSocketManager::ReadResult::MESSAGE_RECEIVED && *read_result != TcpSocketManager::ReadResult::INCOMPLETE_MESSAGE_RECEIVED) {
			LOG_WARN("Invalid data received from client, closing connection", socket_manager_.index(), static_cast<int>(*read_result));
			honorably_disconnect();
			return false;
		}
		return not incoming_queue_.empty();
	} else {
		// If called when the socket is not readable, the client might have disconnected in the meanwhile.
		if (not is_connected()) {
			return false;
		}
		return not incoming_queue().empty();
	}
}
const m2::pb::NetworkMessage* m2::network::ClientManager::peak_incoming_message() {
	if (auto* incoming_queue = get_incoming_queue(); not incoming_queue || incoming_queue->empty()) {
		return nullptr;
	} else {
		return &incoming_queue->front();
	}
}
std::optional<m2::pb::NetworkMessage> m2::network::ClientManager::pop_incoming_message() {
	auto* incoming_queue = get_incoming_queue();
	if (not incoming_queue || incoming_queue->empty()) {
		return std::nullopt;
	}

	auto msg = std::move(incoming_queue->front());
	auto json_str = pb::message_to_json_string(msg);
	LOG_INFO("Popping message from incoming queue of client", socket_manager().index(), json_str->c_str());
	incoming_queue->pop();
	return std::move(msg);
}

bool m2::network::ClientManager::has_outgoing_data() {
	return socket_manager().has_outgoing_data() || not outgoing_queue().empty();
}
void m2::network::ClientManager::queue_outgoing_message(m2::pb::NetworkMessage msg) {
	outgoing_queue().emplace(std::move(msg));
}
void m2::network::ClientManager::send_outgoing_data() {
	auto& socket_manager_ = socket_manager();

	auto send_result = socket_manager_.send_outgoing_data(outgoing_queue());
	if (not send_result) {
		LOG_WARN("Error occurred while writing, closing connection to client", socket_manager_.index(), send_result.error());
		honorably_disconnect();
		return;
	}
	if (*send_result != TcpSocketManager::SendResult::OK) {
		throw M2_ERROR("An invalid or too large outgoing message was queued to client: " + m2::ToString(socket_manager_.index()) + " " + m2::ToString(static_cast<int>(*send_result)));
	}
}
void m2::network::ClientManager::flush_and_shutdown() {
	if (is_ready()) {
		socket_manager().flush(outgoing_queue(), 10000);
	}
	_state = Shutdown{};
}

m2::network::TcpSocketManager& m2::network::ClientManager::socket_manager() {
	return std::visit(m2::overloaded{
		[](Connected& s) -> m2::network::TcpSocketManager& { return s.socket_manager; },
		[](Ready& s) -> m2::network::TcpSocketManager& { return s.socket_manager; },
		[](ReconnectedUntrusted& s) -> m2::network::TcpSocketManager& { return s.socket_manager; },
		[](auto&) -> m2::network::TcpSocketManager& { throw M2_ERROR("Socket unavailable, client is not connected"); },
	}, _state);
}
std::queue<m2::pb::NetworkMessage>* m2::network::ClientManager::get_incoming_queue() {
	return std::visit(m2::overloaded{
		[](Connected& s) -> std::queue<m2::pb::NetworkMessage>* { return &s.incoming_queue; },
		[](Ready& s) -> std::queue<m2::pb::NetworkMessage>* { return &s.incoming_queue; },
		[](ReconnectedUntrusted& s) -> std::queue<m2::pb::NetworkMessage>* { return &s.incoming_queue; },
		[](auto&) -> std::queue<m2::pb::NetworkMessage>* { return nullptr; },
	}, _state);
}
std::queue<m2::pb::NetworkMessage>& m2::network::ClientManager::incoming_queue() {
	return std::visit(m2::overloaded{
		[](Connected& s) -> std::queue<m2::pb::NetworkMessage>& { return s.incoming_queue; },
		[](Ready& s) -> std::queue<m2::pb::NetworkMessage>& { return s.incoming_queue; },
		[](ReconnectedUntrusted& s) -> std::queue<m2::pb::NetworkMessage>& { return s.incoming_queue; },
		[](auto&) -> std::queue<m2::pb::NetworkMessage>& { throw M2_ERROR("Incoming queue unavailable, client is not connected"); },
	}, _state);
}
std::queue<m2::pb::NetworkMessage>& m2::network::ClientManager::outgoing_queue() {
	return std::visit(m2::overloaded{
		[](Ready& s) -> std::queue<m2::pb::NetworkMessage>& { return s.outgoing_queue; },
		[](auto&) -> std::queue<m2::pb::NetworkMessage>& { throw M2_ERROR("Outgoing queue unavailable, client is not ready"); },
	}, _state);
}
