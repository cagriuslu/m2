#pragma once
#include "TcpSocketManager.h"

namespace m2::network {
	class ClientManager {
		std::optional<TcpSocketManager> _socket_manager;
		std::queue<pb::NetworkMessage> _incoming_queue{};
		std::queue<pb::NetworkMessage> _outgoing_queue{};

	public:
		ClientManager(TcpSocket&& socket, int index) : _socket_manager(TcpSocketManager{std::move(socket), index}) {}

		bool is_ready{};

		[[nodiscard]] bool is_still_connected() { return static_cast<bool>(_socket_manager); }
		[[nodiscard]] int fd() { return _socket_manager ? _socket_manager->socket().fd() : -1; }

		/// Returns true if there's some message to process. If the socket is readable, more data will be fetched.
		/// Otherwise only the local buffers will be checked.
		bool has_incoming_data(bool is_socket_readable);
		/// Take a peek at the next fully received message waiting to be processed. Returns nullptr if there are none.
		const pb::NetworkMessage* peak_incoming_message();
		/// Take out the next fully received message waiting to be processed. Returns std::nullopt if there are none.
		std::optional<pb::NetworkMessage> pop_incoming_message();

		[[nodiscard]] bool has_outgoing_data() const;
		/// Place a message in the outgoing message queue to be sent later.
		void queue_outgoing_message(pb::NetworkMessage msg);
		/// Should be called only if the socket is already writeable.
		void send_outgoing_data();

		/// Wait until all outgoing data is sent and shutdown the socket. Ignore if any error occurs.
		void flush_and_shutdown();

	private:
		void clear_socket_and_reset_queues();
	};

	// Filters
	inline bool is_client_ready(const ClientManager& c) { return c.is_ready; }
}