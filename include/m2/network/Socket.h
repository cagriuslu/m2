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

		void_expected bind(uint16_t port);
		void_expected listen(int queue_size);
		void_expected connect(const std::string& ip_addr, uint16_t port);
		expected<Socket> accept();

		/// Returns number of bytes sent. If it's less than the length, the socket out buffer might be full.
		m2::expected<size_t> send(const uint8_t* buffer, size_t length);
		m2::expected<size_t> send(const int8_t* buffer, size_t length) { return send(reinterpret_cast<const uint8_t*>(buffer), length); }
		m2::expected<size_t> send(const char* buffer, size_t length) { return send(reinterpret_cast<const uint8_t*>(buffer), length); }

		m2::expected<size_t> recv(uint8_t* buffer, size_t length);
		m2::expected<size_t> recv(int8_t* buffer, size_t length) { return recv(reinterpret_cast<uint8_t*>(buffer), length); }
		m2::expected<size_t> recv(char* buffer, size_t length) { return recv(reinterpret_cast<uint8_t*>(buffer), length); }

		[[nodiscard]] int fd() const { return _fd; }
	};
}
