#pragma once
#include "../Meta.h"

namespace m2::network {
	namespace detail {
		class PlatformSpecificTcpSocketData;
	}

	class TcpSocket {
		detail::PlatformSpecificTcpSocketData* _platform_specific_data{};
		int _fd{-1};
		in_addr_t _addr{};
		in_port_t _port{};

		explicit TcpSocket(int fd) : _fd(fd) {}
		TcpSocket(int fd, in_addr_t addr, in_port_t port) : _fd(fd), _addr(addr), _port(port) {}

	public:
		static expected<TcpSocket> create();
		TcpSocket(const TcpSocket& other) = delete;
		TcpSocket& operator=(const TcpSocket& other) = delete;
		TcpSocket(TcpSocket&& other) noexcept;
		TcpSocket& operator=(TcpSocket&& other) noexcept;
		~TcpSocket();

		[[nodiscard]] int fd() const { return _fd; }
		[[nodiscard]] std::pair<in_addr_t,in_port_t> address_and_port() const { return std::make_pair(_addr, _port); }

		/// Returns true if successful. Returns false if failed due to EADDRINUSE. Otherwise, returns unexpected.
		expected<bool> bind(uint16_t port);
		void_expected listen(int queue_size);
		/// Tries to connect to the given IP address and port. Returns true if the socket is successfully connected.
		/// Returns false if the connection failed due to external reasons (no route to host, timeout, connection
		/// refused, etc.). Returns unexpected if an unrecoverable error occurs. If unexpected, there's no point
		/// retrying the connection. If false is returned, the same socket can be used to retry connection.
		expected<bool> connect(const std::string& ip_addr, uint16_t port);
		/// Returns another TcpSocket instance that's connected to the client. Returns std::nullopt if the client
		/// connection was aborted by the time it was accepted. Returns unexpected if a socket error occurs.
		expected<std::optional<TcpSocket>> accept();

		/// Returns number of bytes queued into kernel's output buffer. If the output buffer is full, returned value may
		/// be zero, or less than `length`. If -1 is returned, EAGAIN/EWOULDBLOCK has occurred, meaning that the socket
		/// was non-blocking, but the write operation would have blocked, most likely because the kernel's output buffer
		/// is full. If the socket is blocking, the call might block until at least some bytes are sent. If unexpected
		/// is returned, system error message is returned, and the socket shouldn't be used for further communication.
		expected<ssize_t> send(const uint8_t* buffer, size_t length);
		expected<ssize_t> send(const char* buffer, size_t length) { return send(reinterpret_cast<const uint8_t*>(buffer), length); }

		/// Returns the number of bytes actually received. If 0 is returned, EAGAIN/EWOULDBLOCK has occurred. If the
		/// socket was non-blocking, EAGAIN means the receive operation would have blocked because there is no data
		/// available at this time. Or, EAGAIN might mean that receive timeout (SO_RCVTIMEO) has been set on the socket
		/// (regardless of blocking or non-blocking) and it has expired. If receive timeout is not set, and the socket
		/// is blocking, the call would simply block until at least some data is received. If unexpected is returned,
		/// the system error string is returned. In this case, socket should be closed as no further data is expected.
		expected<ssize_t> recv(uint8_t* buffer, size_t length);
		expected<ssize_t> recv(char* buffer, size_t length) { return recv(reinterpret_cast<uint8_t*>(buffer), length); }
	};
}
