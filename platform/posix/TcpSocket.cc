#include <m2/network/TcpSocket.h>
#include <m2/network/Select.h>
#include "PlatformSpecificTcpSocketData.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <m2/Log.h>

m2::expected<m2::network::TcpSocket> m2::network::TcpSocket::create_server(uint16_t port) {
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

    TcpSocket tcp_socket{INADDR_ANY, port};
    tcp_socket._platform_specific_data = new detail::PlatformSpecificTcpSocketData{.fd = socket_result};
    return std::move(tcp_socket);
}

m2::expected<m2::network::TcpSocket> m2::network::TcpSocket::create_client(const std::string& server_ip_addr, uint16_t server_port) {
    int socket_result = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_result == -1) {
        return m2::make_unexpected(strerror(errno));
    }

    TcpSocket tcp_socket{inet_addr(server_ip_addr.c_str()), server_port};
    tcp_socket._platform_specific_data = new detail::PlatformSpecificTcpSocketData{.fd = socket_result};
    return std::move(tcp_socket);
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

m2::expected<bool> m2::network::TcpSocket::bind() {
	sockaddr_in sin{};
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(_port);
	sin.sin_addr.s_addr = _addr;
	int bind_result = ::bind(_platform_specific_data->fd, (sockaddr*) &sin, sizeof(sin));
	if (bind_result == -1) {
		if (errno == EADDRINUSE) {
			return false;
		} else {
			return m2::make_unexpected(strerror(errno));
		}
	}
	return true;
}

m2::void_expected m2::network::TcpSocket::listen(int queue_size) {
	int listen_result = ::listen(_platform_specific_data->fd, queue_size);
	if (listen_result == -1) {
		return m2::make_unexpected(strerror(errno));
	}

	return {};
}

m2::expected<bool> m2::network::TcpSocket::connect() {
	sockaddr_in sin{};
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(_port);
	sin.sin_addr.s_addr = _addr;
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

	TcpSocket child_socket{child_address.sin_addr.s_addr, child_address.sin_port};
    child_socket._platform_specific_data = new detail::PlatformSpecificTcpSocketData{.fd = new_socket};
    return std::move(child_socket);
}

m2::expected<int> m2::network::TcpSocket::send(const uint8_t* buffer, size_t length) {
	auto send_result = ::send(_platform_specific_data->fd, buffer, length, 0);
	if (send_result == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return -1;
		} else {
			return m2::make_unexpected(strerror(errno));
		}
	} else {
		return I(send_result);
	}
}

m2::expected<int> m2::network::TcpSocket::recv(uint8_t* buffer, size_t length) {
	auto recv_result = ::recv(_platform_specific_data->fd, buffer, length, 0);
	if (recv_result == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		}
		return m2::make_unexpected(strerror(errno));
	} else if (recv_result == 0) {
		return m2::make_unexpected("Peer shut connection down");
	} else {
		return I(recv_result);
	}
}
