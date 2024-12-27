#pragma once
#include "IpAddressAndPort.h"
#include "../Meta.h"
#include <cstdio>

namespace m2::network {
	// Forward declarations
	struct Select;
	namespace detail {
		struct PlatformSpecificTcpSocketData;
	}

	class TcpSocket {
		detail::PlatformSpecificTcpSocketData* _platform_specific_data{};
		/// If the socket is a server, the address will contain 0. Otherwise, the address will contain the IP address
		/// of the server.
		IpAddress _addr{};
		/// The port is always the port number of the server.
		Port _port{};

		/// Client sockets held by the server are created with this constructor. Platform specific data should be
		/// initialized after calling this constructor.
		TcpSocket(const IpAddress addr, const Port port) : _addr(addr), _port(port) {}

	public:
        static expected<TcpSocket> create_server(uint16_t port);
        static expected<TcpSocket> create_client(const std::string& server_ip_addr, uint16_t server_port);
		TcpSocket(const TcpSocket& other) = delete;
		TcpSocket& operator=(const TcpSocket& other) = delete;
		TcpSocket(TcpSocket&& other) noexcept;
		TcpSocket& operator=(TcpSocket&& other) noexcept;
		~TcpSocket();

		[[nodiscard]] IpAddressAndPort ip_address_and_port() const { return IpAddressAndPort{_addr, _port}; }

		/// Returns true if successful. Returns false if failed due to EADDRINUSE. Otherwise, returns unexpected.
		expected<bool> bind();
		void_expected listen(int queue_size);
		/// Tries to connect to the server. Returns true if the socket is successfully connected. Returns false if the
		/// connection failed due to external reasons (no route to host, timeout, connection refused, etc.). Returns
		/// unexpected if an unrecoverable error occurs. If unexpected, there's no point retrying the connection. If
		/// false is returned, the same socket can be used to retry connection.
		expected<bool> connect();
		/// Returns another TcpSocket instance that's connected to the client. Returns std::nullopt if the client
		/// connection was aborted by the time it was accepted. Returns unexpected if a socket error occurs.
		expected<std::optional<TcpSocket>> accept();

		/// Returns number of bytes queued into kernel's output buffer. If the output buffer is full, returned value may
		/// be zero, or less than `length`. If -1 is returned, EAGAIN/EWOULDBLOCK has occurred, meaning that the socket
		/// was non-blocking, but the write operation would have blocked, most likely because the kernel's output buffer
		/// is full. If the socket is blocking, the call might block until at least some bytes are sent. If unexpected
		/// is returned, system error message is returned, and the socket shouldn't be used for further communication.
		expected<int> send(const uint8_t* buffer, size_t length);
		expected<int> send(const char* buffer, size_t length) { return send(reinterpret_cast<const uint8_t*>(buffer), length); }

		/// Returns the number of bytes actually received. If 0 is returned, EAGAIN/EWOULDBLOCK has occurred. If the
		/// socket was non-blocking, EAGAIN means the receive operation would have blocked because there is no data
		/// available at this time. Or, EAGAIN might mean that receive timeout (SO_RCVTIMEO) has been set on the socket
		/// (regardless of blocking or non-blocking) and it has expired. If receive timeout is not set, and the socket
		/// is blocking, the call would simply block until at least some data is received. If unexpected is returned,
		/// the system error string is returned. In this case, socket should be closed as no further data is expected.
		expected<int> recv(uint8_t* buffer, size_t length);
		expected<int> recv(char* buffer, size_t length) { return recv(reinterpret_cast<uint8_t*>(buffer), length); }

		// Give Select class access to internals
		friend Select;
	};
}
