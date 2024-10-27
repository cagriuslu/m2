#pragma once
#include "TcpSocketManager.h"
#include "IpAddressAndPort.h"
#include "../sdl/Detail.h"

namespace m2::network {
	class ClientManager {
		// Client is connected but not yet ready
		struct Connected {
			TcpSocketManager socket_manager;
			std::queue<pb::NetworkMessage> incoming_queue{};
		};
		// Client is connected and ready
		struct Ready {
			TcpSocketManager socket_manager;
			std::queue<pb::NetworkMessage> incoming_queue{};
			std::queue<pb::NetworkMessage> outgoing_queue{};
			uint64_t ready_token;
		};
		// Client has disconnected due to network errors
		struct HonorablyDisconnected {
			uint64_t expected_ready_token; // Same as Ready::ready_token
			sdl::ticks_t disconnected_at;
		};
		// A client has reconnected but hasn't provided the correct ready token yet
		struct ReconnectedUntrusted {
			TcpSocketManager socket_manager;
			std::queue<pb::NetworkMessage> incoming_queue{};
			uint64_t expected_ready_token; // Same as Ready::ready_token
			sdl::ticks_t reconnected_at;
		};
		// Client has misbehaved
		struct Misbehaved {};
		// ClientManager has been shutdown
		struct Shutdown {};

		int _index;
		IpAddressAndPort _ip_address_and_port;
		std::variant<Connected, Ready, HonorablyDisconnected, ReconnectedUntrusted, Misbehaved, Shutdown> _state;

	public:
		ClientManager(TcpSocket&& socket, int index);

		// Accessors

		[[nodiscard]] IpAddressAndPort ip_address_and_port() const { return _ip_address_and_port; }
		[[nodiscard]] bool is_connected() const;
		[[nodiscard]] int fd();
		[[nodiscard]] bool is_ready() const;
		[[nodiscard]] bool is_disconnected() const;
		[[nodiscard]] bool is_untrusted() const;
		[[nodiscard]] bool is_disconnected_or_untrusted() const;
		[[nodiscard]] std::optional<sdl::ticks_t> disconnected_or_untrusted_since() const;
		[[nodiscard]] bool has_misbehaved() const;

		// Modifiers

		/// Returns true if this is the first time the client becomes ready, or the token matches a previous one.
		/// Returns false if the token mismatches the previous value.
		bool set_ready_token(uint64_t ready_token);
		/// Honorably disconnect the client connect. The client is allowed to reconnect again.
		void honorably_disconnect();
		/// If a new candidate is connected, it's initially untrusted until the correct ready_token is received.
		void untrusted_client_reconnected(TcpSocket&&);
		/// Once misbehaved, there's no turning back.
		void set_misbehaved();

		/// Returns true if there's some message to process. If the socket is readable, more data will be fetched.
		/// Otherwise only the local buffers will be checked.
		bool has_incoming_data(bool is_socket_readable);
		/// Take a peek at the next fully received message waiting to be processed. Returns nullptr if there are none.
		const pb::NetworkMessage* peak_incoming_message();
		/// Take out the next fully received message waiting to be processed. Returns std::nullopt if there are none.
		std::optional<pb::NetworkMessage> pop_incoming_message();

		[[nodiscard]] bool has_outgoing_data();
		/// Place a message in the outgoing message queue to be sent later.
		void queue_outgoing_message(pb::NetworkMessage msg);
		/// Should be called only if the socket is already writeable.
		void send_outgoing_data();

		/// Wait until all outgoing data is sent and shutdown the socket. Ignore if any error occurs.
		void flush_and_shutdown();

	private:
		TcpSocketManager& socket_manager();
		std::queue<pb::NetworkMessage>* get_incoming_queue();
		std::queue<pb::NetworkMessage>& incoming_queue();
		std::queue<pb::NetworkMessage>& outgoing_queue();
	};

	// Filters
	inline bool is_client_ready(const ClientManager& c) { return c.is_ready(); }
}