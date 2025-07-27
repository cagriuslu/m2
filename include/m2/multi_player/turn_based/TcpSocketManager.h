#pragma once
#include "../../network/TcpSocket.h"
#include "../../Meta.h"
#include <Network.pb.h>
#include <array>
#include <queue>

namespace m2::network {
	/// \brief Allows sending and receiving of discrete messages (instead of individual bytes) over a TCP socket.
	/// \details For TurnBased networking model, clients use the manager directly, while the server utilizes client
	/// manager to hold the socket manager.
	class TcpSocketManager {
		TcpSocket _socket;
		int _index; // Used for logging

		std::array<char, 65536> _incoming_buffer{};
		size_t _incoming_buffer_next_available_position{};

		std::array<char, 65536> _outgoing_buffer{};
		size_t _outgoing_buffer_start_position{};
		size_t _outgoing_buffer_bytes_left{};

	public:
		TcpSocketManager(TcpSocket&& s, int index) : _socket(std::move(s)), _index(index) {}

		TcpSocket& socket() { return _socket; }
		[[nodiscard]] int index() const { return _index; }

		enum class ReadResult {
			MESSAGE_RECEIVED, /// At least one full message is available in incoming_queue
			INCOMPLETE_MESSAGE_RECEIVED, /// Some bytes have received but a whole message is not yet available
			INVALID_MESSAGE, /// A full message is received, but parsing of it has failed
			BUFFER_OVERFLOW, /// Buffer has overflown before a whole message can be built
			GAME_HASH_MISMATCH, // Received game hash doesn't match
		};
		/// Should be called only if the socket is already readable (ex. select).
		/// Returns ReadResult if expected, or the system error message if unexpected.
		m2::expected<ReadResult> read_incoming_data(std::queue<pb::TurnBasedNetworkMessage>& read_to);

		[[nodiscard]] bool has_outgoing_data() const { return _outgoing_buffer_bytes_left; }

		enum class SendResult {
			OK, /// No errors
			INVALID_MESSAGE, /// A message in the outgoing queue failed to be serialized
			BUFFER_WOULD_OVERFLOW, /// A message in the outgoing queue would have overflow the buffer
		};
		/// Send outgoing data if exists. Should be called only if the socket is already writeable (ex. select).
		/// Returns the system error message if unexpected.
		expected<SendResult> send_outgoing_data(std::queue<pb::TurnBasedNetworkMessage>& read_from);

		/// Wait until all outgoing data is sent and ignore if any error occurs.
		void flush(std::queue<pb::TurnBasedNetworkMessage>& read_from, int timeout_ms);
	};
}
