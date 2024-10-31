#include <m2/network/TcpSocket.h>
#include <m2/network/Select.h>
#include "PlatformSpecificTcpSocketData.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <m2/Log.h>

m2::network::TcpSocket::TcpSocket(int fd)
	: _platform_specific_data(new detail::PlatformSpecificTcpSocketData{.fd = fd}) {}
m2::network::TcpSocket::TcpSocket(int fd, IpAddress addr, Port port)
	: _platform_specific_data(new detail::PlatformSpecificTcpSocketData{.fd = fd}), _addr(addr), _port(port) {}

m2::expected<m2::network::TcpSocket> m2::network::TcpSocket::create() {
	int socket_result = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_result == -1) {
		return m2::make_unexpected(strerror(errno));
	}

	// Enable linger for 10 seconds, because when the game is finished, the server sends the final ServerUpdate and
	// immediately closes the connection
	linger l{.l_onoff = 1, .l_linger = 10};
	if (setsockopt(socket_result, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) == -1) {
		LOG_WARN("Unable to enable TCP linger", std::string(strerror(errno)));
	}

	return TcpSocket{socket_result};
}

m2::network::TcpSocket::TcpSocket(TcpSocket&& other) noexcept {
	*this = std::move(other);
}

m2::network::TcpSocket& m2::network::TcpSocket::operator=(TcpSocket&& other) noexcept {
	std::swap(_platform_specific_data, other._platform_specific_data);
	std::swap(_addr, other._addr);
	std::swap(_port, other._port);
	return *this;
}

m2::network::TcpSocket::~TcpSocket() {
	if (_platform_specific_data) {
		if (0 <= _platform_specific_data->fd) {
			close(_platform_specific_data->fd);
		}
		delete _platform_specific_data;
	}
}

m2::expected<bool> m2::network::TcpSocket::bind(uint16_t port) {
	sockaddr_in sin{};
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;
	int bind_result = ::bind(_platform_specific_data->fd, (sockaddr*) &sin, sizeof(sin));
	if (bind_result == -1) {
		if (errno == EADDRINUSE) {
			return false;
		} else {
			return m2::make_unexpected(strerror(errno));
		}
	}

	_port = port;
	return true;
}

m2::void_expected m2::network::TcpSocket::listen(int queue_size) {
	int listen_result = ::listen(_platform_specific_data->fd, queue_size);
	if (listen_result == -1) {
		return m2::make_unexpected(strerror(errno));
	}

	return {};
}

m2::expected<bool> m2::network::TcpSocket::connect(const std::string& ip_addr, uint16_t port) {
	sockaddr_in sin{};
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr(ip_addr.c_str());
	int connect_result = ::connect(_platform_specific_data->fd, (sockaddr*) &sin, sizeof(sin));
	if (connect_result == -1) {
		if (errno == ECONNREFUSED || errno == EHOSTUNREACH || errno == ENETUNREACH || errno == ETIMEDOUT) {
			// Connection failed due to reasons not under our control
			return false;
		}
		return m2::make_unexpected(strerror(errno));
	}
	return true;
}

m2::expected<std::optional<m2::network::TcpSocket>> m2::network::TcpSocket::accept() {
	sockaddr_in child_address{};
	socklen_t child_address_len{};
	int new_socket = ::accept(_platform_specific_data->fd, (sockaddr*) &child_address, &child_address_len);
	if (new_socket == -1) {
		if (errno == ECONNABORTED) {
			return std::nullopt;
		}
		return m2::make_unexpected(strerror(errno));
	}
	return TcpSocket{new_socket, child_address.sin_addr.s_addr, child_address.sin_port};
}

m2::expected<ssize_t> m2::network::TcpSocket::send(const uint8_t* buffer, size_t length) {
	auto send_result = ::send(_platform_specific_data->fd, buffer, length, 0);
	if (send_result == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return -1;
		} else {
			return m2::make_unexpected(strerror(errno));
		}
	} else {
		return send_result;
	}
}

m2::expected<ssize_t> m2::network::TcpSocket::recv(uint8_t* buffer, size_t length) {
	auto recv_result = ::recv(_platform_specific_data->fd, buffer, length, 0);
	if (recv_result == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		}
		return m2::make_unexpected(strerror(errno));
	} else if (recv_result == 0) {
		return m2::make_unexpected("Peer shut connection down");
	} else {
		return recv_result;
	}
}
