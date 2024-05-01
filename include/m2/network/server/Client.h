#pragma once
#include "../Socket.h"
#include "../../Meta.h"
#include <queue>

namespace m2::network::server {
	class Client {
		std::optional<Socket> _socket;
		bool _is_ready{};
		std::optional<pb::NetworkMessage> _incoming_msg;
		std::queue<pb::NetworkMessage> _outgoing_queue;

	public:
		explicit Client(Socket&& s) : _socket(std::move(s)) {}

		/// Returns if the client is still connected.
		/// Once the lobby is closed, a client can disconnect. This object is kept in case the client comes back.
		[[nodiscard]] bool is_still_connected() const { return static_cast<bool>(_socket); }
		Socket& socket() { return *_socket; }
		void set_socket(Socket&& s) { _socket = std::move(s); }
		void clear_socket() { _socket.reset(); }

		/// Returns if the client is ready.
		/// Clients report their sender_id if they are ready. Or clear it if they become unready.
		[[nodiscard]] bool is_ready() const { return _is_ready; }
		void set_ready(bool state) { _is_ready = state; }
		void clear_ready() { _is_ready = false; }

		m2::expected<std::optional<pb::NetworkMessage>> save_incoming_message(char* read_buffer, size_t read_buffer_length);
		std::optional<pb::NetworkMessage> peak_incoming_message();
		std::optional<pb::NetworkMessage> pop_incoming_message();
		void push_outgoing_message(pb::NetworkMessage&& msg);
		expected<bool> flush_outgoing_messages();
	};

	// Filters
	inline bool is_client_ready(const Client& c) { return c.is_ready(); }
}
