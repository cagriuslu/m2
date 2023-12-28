#include <m2/network/Socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

m2::expected<m2::network::Socket> m2::network::Socket::create_tcp() {
	int socket_result = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_result == -1) {
		return m2::make_unexpected(strerror(errno));
	}

	return Socket{socket_result};
}

m2::expected<m2::network::Socket> m2::network::Socket::create_udp() {
	int socket_result = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socket_result == -1) {
		return m2::make_unexpected(strerror(errno));
	}

	return Socket{socket_result};
}

m2::network::Socket::Socket(Socket&& other) noexcept {
	*this = std::move(other);
}

m2::network::Socket& m2::network::Socket::operator=(Socket&& other) noexcept {
	std::swap(_fd, other._fd);
	std::swap(_addr, other._addr);
	std::swap(_port, other._port);
	return *this;
}

m2::network::Socket::~Socket() {
	if (0 <= _fd) {
		close(_fd);
	}
}

m2::void_expected m2::network::Socket::bind(uint16_t port) {
	sockaddr_in sin{};
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;
	int bind_result = ::bind(_fd, (sockaddr*) &sin, sizeof(sin));
	if (bind_result == -1) {
		return m2::make_unexpected(strerror(errno));
	}

	_port = port;
	return {};
}

m2::void_expected m2::network::Socket::listen(int queue_size) {
	int listen_result = ::listen(_fd, queue_size);
	if (listen_result == -1) {
		return m2::make_unexpected(strerror(errno));
	}

	return {};
}

m2::void_expected m2::network::Socket::connect(const std::string& ip_addr, uint16_t port) {
	sockaddr_in sin{};
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr(ip_addr.c_str());
	int connect_result = ::connect(_fd, (sockaddr*) &sin, sizeof(sin));
	if (connect_result == -1) {
		return m2::make_unexpected(strerror(errno));
	}

	return {};
}

m2::expected<m2::network::Socket> m2::network::Socket::accept() {
	sockaddr_in child_address{};
	socklen_t child_address_len{};
	int new_socket = ::accept(_fd, (sockaddr*) &child_address, &child_address_len);
	if (new_socket == -1) {
		return m2::make_unexpected(strerror(errno));
	}

	return Socket{new_socket, child_address.sin_addr.s_addr, child_address.sin_port};
}

m2::expected<size_t> m2::network::Socket::send(const uint8_t* buffer, size_t length) {
	auto send_result = ::send(_fd, buffer, length, 0);
	if (send_result == -1) {
		if (errno == EAGAIN) {
			return 0;
		} else {
			return m2::make_unexpected(strerror(errno));
		}
	} else {
		return static_cast<size_t>(send_result);
	}
}

m2::expected<size_t> m2::network::Socket::recv(uint8_t* buffer, size_t length) {
	auto recv_result = ::recv(_fd, buffer, length, 0);
	if (recv_result == -1) {
		if (errno == EAGAIN) {
			return 0;
		} else {
			return m2::make_unexpected(strerror(errno));
		}
	} else {
		return static_cast<size_t>(recv_result);
	}
}
