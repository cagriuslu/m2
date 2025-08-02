#include <m2/network/TcpSocket.h>
#include <m2/network/Select.h>
#include "PlatformSpecificSocketData.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <m2/Log.h>

m2::expected<m2::network::TcpSocket> m2::network::TcpSocket::CreateServerSideSocket(const uint16_t port) {
    const auto socket_result = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_result == -1) {
        return make_unexpected(strerror(errno));
    }

    // Enable linger for 10 seconds, because when the game is finished, the server sends the final TurnBasedServerUpdate and
    // immediately closes the connection. The socket should linger for some time for the final TurnBasedServerUpdate to be sent.
    constexpr linger l{.l_onoff = 1, .l_linger = 10};
    if (setsockopt(socket_result, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) == -1) {
        LOG_WARN("Unable to enable TCP linger", std::string(strerror(errno)));
    }

    TcpSocket tcp_socket;
	tcp_socket._serverAddr = INADDR_ANY;
	tcp_socket._serverPort = port;
    tcp_socket._platform_specific_data = new detail::PlatformSpecificSocketData{.fd = socket_result};
    return std::move(tcp_socket);
}
m2::expected<m2::network::TcpSocket> m2::network::TcpSocket::CreateClientSideSocket(const std::string& server_ip_addr, const uint16_t server_port) {
    const auto socket_result = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_result == -1) {
        return make_unexpected(strerror(errno));
    }

    TcpSocket tcp_socket;
	tcp_socket._serverAddr = ToIpAddress(server_ip_addr);
	tcp_socket._serverPort = server_port;
    tcp_socket._platform_specific_data = new detail::PlatformSpecificSocketData{.fd = socket_result};
    return std::move(tcp_socket);
}
m2::network::TcpSocket::TcpSocket(TcpSocket&& other) noexcept {
	*this = std::move(other);
}
m2::network::TcpSocket& m2::network::TcpSocket::operator=(TcpSocket&& other) noexcept {
	std::swap(_platform_specific_data, other._platform_specific_data);
	std::swap(_serverAddr, other._serverAddr);
	std::swap(_clientAddr, other._clientAddr);
	std::swap(_serverPort, other._serverPort);
	std::swap(_clientPort, other._clientPort);
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
	if (not IsServerSideListeningSocket()) {
		throw M2_ERROR("Bind called on a non-listening non-server socket");
	}

	sockaddr_in sin{};
#ifdef __APPLE__
	sin.sin_len = sizeof(sin);
#endif
	sin.sin_family = AF_INET;
	sin.sin_port = htons(_serverPort);
	sin.sin_addr.s_addr = _serverAddr;
	int bind_result = ::bind(_platform_specific_data->fd, reinterpret_cast<sockaddr*>(&sin), sizeof(sin));
	if (bind_result == -1) {
		if (errno == EADDRINUSE) {
			return false;
		}
		return make_unexpected(strerror(errno));
	}
	return true;
}

m2::void_expected m2::network::TcpSocket::listen(const int queue_size) {
	if (not IsServerSideListeningSocket()) {
		throw M2_ERROR("Listen called on a non-listening non-server socket");
	}

	if (const auto listen_result = ::listen(_platform_specific_data->fd, queue_size); listen_result == -1) {
		return make_unexpected(strerror(errno));
	}

	return {};
}

m2::expected<bool> m2::network::TcpSocket::connect() {
	if (not IsClientSideSocket()) {
		throw M2_ERROR("Connect called on a non-client socket");
	}

	sockaddr_in sin{};
#ifdef __APPLE__
	sin.sin_len = sizeof(sin);
#endif
	sin.sin_family = AF_INET;
	sin.sin_port = htons(_serverPort);
	sin.sin_addr.s_addr = _serverAddr;
	if (const auto connect_result = ::connect(_platform_specific_data->fd, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)); connect_result == -1) {
		if (errno == ECONNREFUSED || errno == EHOSTUNREACH || errno == ENETUNREACH || errno == ETIMEDOUT) {
			// Connection failed due to reasons not under our control
			return false;
		}
		return make_unexpected(strerror(errno));
	}
	return true;
}

m2::expected<std::optional<m2::network::TcpSocket>> m2::network::TcpSocket::accept() {
	if (not IsServerSideListeningSocket()) {
		throw M2_ERROR("Accept called on a non-listening non-server socket");
	}

	sockaddr child_address{};
	socklen_t child_address_len = sizeof(sockaddr);
	int new_socket = ::accept(_platform_specific_data->fd, &child_address, &child_address_len);
	if (new_socket == -1) {
		if (errno == ECONNABORTED) {
			return std::nullopt;
		}
		return make_unexpected(strerror(errno));
	}

	TcpSocket child_socket;
	child_socket._clientAddr = reinterpret_cast<sockaddr_in*>(&child_address)->sin_addr.s_addr;
	child_socket._serverPort = _serverPort;
	child_socket._clientPort = reinterpret_cast<sockaddr_in*>(&child_address)->sin_port;
    child_socket._platform_specific_data = new detail::PlatformSpecificSocketData{.fd = new_socket};
    return std::move(child_socket);
}

m2::expected<int> m2::network::TcpSocket::send(const uint8_t* buffer, const size_t length) {
	if (not IsServerSideConnectedSocket() && not IsClientSideSocket()) {
		throw M2_ERROR("Send called on a non-connected socket");
	}

	const auto send_result = ::send(_platform_specific_data->fd, buffer, length, 0);
	if (send_result == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return -1;
		}
		return make_unexpected(strerror(errno));
	}
	return I(send_result);
}

m2::expected<int> m2::network::TcpSocket::recv(uint8_t* buffer, const size_t length) {
	if (not IsServerSideConnectedSocket() && not IsClientSideSocket()) {
		throw M2_ERROR("Recv called on a non-connected socket");
	}

	const auto recv_result = ::recv(_platform_specific_data->fd, buffer, length, 0);
	if (recv_result == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		}
		return make_unexpected(strerror(errno));
	}
	if (recv_result == 0) {
		return make_unexpected("Peer shut connection down");
	}
	return I(recv_result);
}
