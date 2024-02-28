#pragma once
#include "../Meta.h"

namespace m2::network {
	class Socket {
		int _fd{-1};
		in_addr_t _addr{};
		in_port_t _port{};

		explicit Socket(int fd) : _fd(fd) {}
		Socket(int fd, in_addr_t addr, in_port_t port) : _fd(fd), _addr(addr), _port(port) {}

	public:
		static expected<Socket> create_tcp();
		static expected<Socket> create_udp();
		Socket(const Socket& other) = delete;
		Socket& operator=(const Socket& other) = delete;
		Socket(Socket&& other) noexcept;
		Socket& operator=(Socket&& other) noexcept;
		~Socket();

		/// Returns true if successful. Returns false if failed due to EADDRINUSE. Otherwise, returns unexpected.
		expected<bool> bind(uint16_t port);
		void_expected listen(int queue_size);
		void_expected connect(const std::string& ip_addr, uint16_t port);
		expected<Socket> accept();

		/// Returns number of bytes sent. If it's less than the length, the socket out buffer might be full.
		/// If it's -1, EAGAIN has occured. Otherwise, error string is returned.
		expected<ssize_t> send(const uint8_t* buffer, size_t length);
		expected<ssize_t> send(const int8_t* buffer, size_t length) { return send(reinterpret_cast<const uint8_t*>(buffer), length); }
		expected<ssize_t> send(const char* buffer, size_t length) { return send(reinterpret_cast<const uint8_t*>(buffer), length); }

		/// Returns the number of bytes received. If it's 0, the socket is shut down from the other side.
		/// If it's -1, EAGAIN has occured. Otherwise, error string is returned.
		expected<ssize_t> recv(uint8_t* buffer, size_t length);
		expected<ssize_t> recv(int8_t* buffer, size_t length) { return recv(reinterpret_cast<uint8_t*>(buffer), length); }
		expected<ssize_t> recv(char* buffer, size_t length) { return recv(reinterpret_cast<uint8_t*>(buffer), length); }

		[[nodiscard]] int fd() const { return _fd; }
	};
}
