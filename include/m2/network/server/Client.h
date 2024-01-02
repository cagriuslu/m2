#pragma once
#include "../Socket.h"
#include "../../Meta.h"
#include <queue>

namespace m2::network::server {
	class Client {
		std::optional<Socket> _socket;
		std::optional<int32_t> _sender_id;
		std::queue<pb::NetworkMessage> _incoming_queue, _outgoing_queue;

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
		[[nodiscard]] bool is_ready() const { return static_cast<bool>(_sender_id); }
		[[nodiscard]] int32_t sender_id() const { return *_sender_id; }
		void set_ready(int32_t sender_id) { _sender_id = sender_id; }
		void clear_ready() { _sender_id.reset(); }

		void_expected fetch_incoming_messages(char* read_buffer, size_t read_buffer_length);
		void_expected flush_outgoing_messages();
		std::optional<pb::NetworkMessage> pop_incoming_message();
		void push_outgoing_message(pb::NetworkMessage&& msg);
	};
}
