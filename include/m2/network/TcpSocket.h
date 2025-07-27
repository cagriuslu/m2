#pragma once
#include "IpAddressAndPort.h"
#include "../Meta.h"
#include <cstdio>

namespace m2::network {
	// Forward declarations
	struct Select;
	namespace detail {
		struct PlatformSpecificSocketData;
	}

	/// \brief Platform-agnostic TCP socket object.
	class TcpSocket {
		detail::PlatformSpecificSocketData* _platform_specific_data{};
		IpAddress _serverAddr{}, _clientAddr{};
		Port _serverPort{}, _clientPort{};

		/// \details Platform specific data should be initialized after calling this constructor.
		TcpSocket() = default;

	public:
        static expected<TcpSocket> CreateServerSideSocket(uint16_t port);
        static expected<TcpSocket> CreateClientSideSocket(const std::string& server_ip_addr, uint16_t server_port);
		TcpSocket(const TcpSocket& other) = delete;
		TcpSocket& operator=(const TcpSocket& other) = delete;
		TcpSocket(TcpSocket&& other) noexcept;
		TcpSocket& operator=(TcpSocket&& other) noexcept;
		~TcpSocket();

		// Accessors

		[[nodiscard]] IpAddressAndPort GetServerIpAddressAndPort() const { return IpAddressAndPort{_serverAddr, _serverPort}; }
		[[nodiscard]] IpAddressAndPort GetClientIpAddressAndPort() const { return IpAddressAndPort{_clientAddr, _clientPort}; }
		[[nodiscard]] bool IsServerSideListeningSocket() const { return _serverAddr == 0 && _clientAddr == 0 && _serverPort && _clientPort == 0; }
		[[nodiscard]] bool IsServerSideConnectedSocket() const { return _serverAddr == 0 && _clientAddr && _serverPort && _clientPort; }
		[[nodiscard]] bool IsClientSideSocket() const { return _serverAddr && _clientAddr == 0 && _serverPort && _clientPort == 0; }

		// Modifiers

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
		expected<int> send(const char* buffer, const size_t length) { return send(reinterpret_cast<const uint8_t*>(buffer), length); }

		/// Returns the number of bytes actually received. If 0 is returned, EAGAIN/EWOULDBLOCK has occurred. If the
		/// socket was non-blocking, EAGAIN means the receive operation would have blocked because there is no data
		/// available at this time. Or, EAGAIN might mean that receive timeout (SO_RCVTIMEO) has been set on the socket
		/// (regardless of blocking or non-blocking) and it has expired. If receive timeout is not set, and the socket
		/// is blocking, the call would simply block until at least some data is received. If unexpected is returned,
		/// the system error string is returned. In this case, socket should be closed as no further data is expected.
		expected<int> recv(uint8_t* buffer, size_t length);
		expected<int> recv(char* buffer, const size_t length) { return recv(reinterpret_cast<uint8_t*>(buffer), length); }

		/// Give Select class access to the internals
		friend Select;
	};
}
