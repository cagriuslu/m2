#pragma once
#include "../Socket.h"
#include "../../Meta.h"
#include <queue>

namespace m2::network::server {
	class Client {
		std::optional<Socket> _socket;
		int _index; // Used for logging
		bool _is_ready{};

		std::array<char, 65536> _incoming_buffer{};
		size_t _incoming_buffer_next_available_position{};
		std::queue<pb::NetworkMessage> _incoming_queue;

		std::queue<pb::NetworkMessage> _outgoing_queue;
		std::array<char, 65536> _outgoing_buffer{};
		size_t _outgoing_buffer_start_position{};
		size_t _outgoing_buffer_bytes_left{};

	public:
		explicit Client(Socket&& s, int index) : _socket(std::move(s)), _index(index) {}

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

		enum class ReadResult {
			MESSAGE_RECEIVED, /// At least one full message is available in incoming_queue
			INCOMPLETE_MESSAGE_RECEIVED, /// Some bytes have received but a whole message is not yet available
			INVALID_MESSAGE, /// A full message is received, but parsing of it has failed
			BUFFER_OVERFLOW, /// Buffer has overflown before a whole message can be built
			GAME_HASH_MISMATCH, // Received game hash doesn't match
		};
		/// Should be called only if the socket is already readable (ex. select).
		/// Returns ReadResult if expected, or the system error message if unexpected.
		m2::expected<ReadResult> read_incoming_data();

		/// Take a peek at the next fully received message waiting to be processed. Returns nullptr if there are none.
		const pb::NetworkMessage* peak_incoming_message();
		/// Take out the next fully received message waiting to be processed. Returns std::nullopt if there are none.
		std::optional<pb::NetworkMessage> pop_incoming_message();

		/// Place a message in the outgoing message queue to be sent later.
		void queue_outgoing_message(pb::NetworkMessage msg);

		enum class SendResult {
			OK, /// No errors
			INVALID_MESSAGE, /// A message in the outgoing queue failed to be serialized
			BUFFER_WOULD_OVERFLOW, /// A message in the outgoing queue would have overflow the buffer
		};
		/// Send outgoing data if exists. Should be called only if the socket is already writeable (ex. select).
		/// Returns the system error message if unexpected.
		expected<SendResult> send_outgoing_data();

		/// Wait until all outgoing data is sent and shutdown the socket. Ignore if any error occurs.
		void flush_and_shutdown(int timeout_ms);
	};

	// Filters
	inline bool is_client_ready(const Client& c) { return c.is_ready(); }
}
