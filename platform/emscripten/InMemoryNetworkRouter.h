#pragma once
#include <m2/network/Types.h>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace m2::network::detail {
	struct InMemoryDatagram {
		IpAddressAndPort sourceAddressAndPort;
		std::vector<uint8_t> payload;
	};
	/// Global in-memory UDP router. Server and client sockets register an inbox at their own port. Send() delivers a
	/// the payload to the destination's inbox.
	class InMemoryUdpRouter {
		std::map<uint16_t, std::deque<InMemoryDatagram>> _inboxesByNetworkOrderPort;
		uint16_t _nextEphemeralHostOrderPort{49152}; /// Start of the IANA dynamic/ephemeral port range.

		InMemoryUdpRouter() = default;

	public:
		static InMemoryUdpRouter& Instance();

		void RegisterInbox(uint16_t networkOrderPort);
		/// Allocates an unused synthetic ephemeral port, registers an empty inbox at it, and returns its Port.
		Port AllocateInbox();
		void UnregisterInbox(uint16_t networkOrderPort);

		/// Pushes a copy of the datagram into the destination's inbox. If no inbox is registered (nobody is listening
		/// on that port) the datagram is dropped.
		void Deliver(uint16_t destinationNetworkOrderPort, const InMemoryDatagram& datagram);
		std::deque<InMemoryDatagram>* TryGetInbox(uint16_t networkOrderPort);
	};

	/// Bidirectional TCP byte stream shared by the two ends of a connection
	struct InMemoryTcpConnection {
		std::deque<uint8_t> clientToServerBytes;
		std::deque<uint8_t> serverToClientBytes;
		bool clientEndOpen{true};
		bool serverEndOpen{true};
	};
	/// A connection initiated by a client (via connect()) but not yet accepted by the server (via accept()).
	struct InMemoryTcpPendingConnection {
		IpAddressAndPort clientAddressAndPort;
		std::shared_ptr<InMemoryTcpConnection> connection;
	};
	/// Global in-memory TCP rendezvous point. This class doesn't carry data, it only facilitates connection. A server
	/// registers its port, a client connects to that port, and the server later accepts the queued connection.
	class InMemoryTcpRouter {
		/// An empty deque signifies that a server is listening on that port
		std::map<uint16_t, std::deque<InMemoryTcpPendingConnection>> _connectionsPendingAcceptanceByNetworkOrderListenPort;
		uint16_t _nextEphemeralHostOrderPort{49152}; // Start of the IANA dynamic/ephemeral port range.

		InMemoryTcpRouter() = default;

	public:
		static InMemoryTcpRouter& Instance();

		void RegisterListener(uint16_t networkOrderListenPort);
		void UnregisterListener(uint16_t networkOrderListenPort);
		[[nodiscard]] bool IsListening(uint16_t networkOrderListenPort) const;

		/// Initiates a connection if a listener exist at the given port. If a listener is registered, a fresh
		/// InMemoryTcpPendingConnection is created and queued for the server to accept. This action also assigns a
		/// synthetic client address+port assigned to the client, which is written into clientAddressAndPort. Returns
		/// nullptr if no listener is registered (connection refused), so the caller may retry later.
		std::shared_ptr<InMemoryTcpConnection> Connect(uint16_t networkOrderListenPort, IpAddressAndPort& clientAddressAndPort);
		/// Pops and returns the next pending connection at the listener's port, or nullopt if none is pending.
		std::optional<InMemoryTcpPendingConnection> Accept(uint16_t networkOrderListenPort);
		/// Returns true if at least one connection is pending acceptance at the listener's (network-order) port.
		[[nodiscard]] bool HasPendingConnection(uint16_t networkOrderListenPort) const;
	};
}
