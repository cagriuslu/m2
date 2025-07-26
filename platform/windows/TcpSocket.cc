#include <m2/network/TcpSocket.h>
#include "PlatformSpecificTcpSocketData.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <mutex>

#pragma comment(lib, "Ws2_32.lib")

std::once_flag g_initialize_winsock_once;
WSADATA g_wsa_data;
// TODO call WSACleanup() before shutting down the program

using namespace m2;
using namespace m2::network;

expected<TcpSocket> TcpSocket::CreateServerSideSocket(uint16_t port) {
    std::call_once(g_initialize_winsock_once, []() {
        // Initialize WinSock2
        if (int result = WSAStartup(MAKEWORD(2,2), &g_wsa_data); result != 0) {
            throw M2_ERROR("WSAStartup failed: " + m2::ToString(result));
        }
    });

    addrinfo hints{};
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    addrinfo *result = nullptr;
    auto addr_error_code = ::getaddrinfo(nullptr, m2::ToString(port).c_str(), &hints, &result);
    if (addr_error_code) {
        return m2::make_unexpected("getaddrinfo failed: " + m2::ToString(addr_error_code));
    }

    // Create the socket
    SOCKET listen_socket = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listen_socket == INVALID_SOCKET) {
        auto last_error = WSAGetLastError();
        freeaddrinfo(result);
        return m2::make_unexpected("socket failed: " + m2::ToString(last_error));
    }

    TcpSocket tcp_socket;
    tcp_socket._serverAddr = INADDR_ANY;
    tcp_socket._serverPort = port;
    tcp_socket._platform_specific_data = new detail::PlatformSpecificTcpSocketData{.address_info = result, .socket = listen_socket};
    return std::move(tcp_socket);
}

expected<TcpSocket> TcpSocket::CreateClientSideSocket(const std::string& server_ip_addr, uint16_t server_port) {
    std::call_once(g_initialize_winsock_once, []() {
        // Initialize WinSock2
        if (int result = WSAStartup(MAKEWORD(2,2), &g_wsa_data); result != 0) {
            throw M2_ERROR("WSAStartup failed: " + m2::ToString(result));
        }
    });

    addrinfo hints{};
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    addrinfo *result = nullptr;
    auto addr_error_code = ::getaddrinfo(server_ip_addr.c_str(), m2::ToString(server_port).c_str(), &hints, &result);
    if (addr_error_code) {
        return m2::make_unexpected("getaddrinfo failed: " + m2::ToString(addr_error_code));
    }

    // Create the socket using the first returned address result
    SOCKET connect_socket = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (connect_socket == INVALID_SOCKET) {
        auto last_error = WSAGetLastError();
        freeaddrinfo(result);
        return m2::make_unexpected("socket failed: " + m2::ToString(last_error));
    }

    TcpSocket tcp_socket;
    tcp_socket._serverAddr = reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr.S_un.S_addr;
    tcp_socket._serverPort = server_port;
    tcp_socket._platform_specific_data = new detail::PlatformSpecificTcpSocketData{.address_info = result, .socket = connect_socket};
    return std::move(tcp_socket);
}

TcpSocket::TcpSocket(TcpSocket&& other) noexcept {
    *this = std::move(other);
}

TcpSocket& TcpSocket::operator=(TcpSocket&& other) noexcept {
    std::swap(_platform_specific_data, other._platform_specific_data);
    std::swap(_serverAddr, other._serverAddr);
    std::swap(_clientAddr, other._clientAddr);
    std::swap(_serverPort, other._serverPort);
    std::swap(_clientPort, other._clientPort);
    return *this;
}

TcpSocket::~TcpSocket() {
    if (_platform_specific_data) {
        if (_platform_specific_data->address_info) {
            freeaddrinfo(_platform_specific_data->address_info);
        }
        if (_platform_specific_data->socket) {
            closesocket(_platform_specific_data->socket);
        }
        delete _platform_specific_data;
    }
}

expected<bool> TcpSocket::bind() {
    if (not IsServerSideListeningSocket()) {
        throw M2_ERROR("Bind called on a non-listening non-server socket");
    }

    // Bind the socket
    auto bind_result = ::bind(_platform_specific_data->socket, _platform_specific_data->address_info->ai_addr, static_cast<int>(_platform_specific_data->address_info->ai_addrlen));
    if (bind_result == SOCKET_ERROR) {
        auto last_error = WSAGetLastError();
        if (last_error == WSAEADDRINUSE) {
            return false;
        } else {
            return m2::make_unexpected("bind failed: " + m2::ToString(last_error));
        }
    }
    return true;
}

void_expected TcpSocket::listen(int queue_size) {
    if (not IsServerSideListeningSocket()) {
        throw M2_ERROR("Listen called on a non-listening non-server socket");
    }

    if (::listen(_platform_specific_data->socket, queue_size) == SOCKET_ERROR) {
        return m2::make_unexpected("listen failed: " + m2::ToString(WSAGetLastError()));
    }

    return {};
}

expected<bool> TcpSocket::connect() {
    if (not IsClientSideSocket()) {
        throw M2_ERROR("Connect called on a non-client socket");
    }

    int connect_result = ::connect(_platform_specific_data->socket, _platform_specific_data->address_info->ai_addr, static_cast<int>(_platform_specific_data->address_info->ai_addrlen));
    if (connect_result == SOCKET_ERROR) {
        auto last_error = WSAGetLastError();
        if (last_error == WSAECONNREFUSED || last_error == WSAENETUNREACH || last_error == WSAEHOSTUNREACH || last_error == WSAETIMEDOUT) {
            return false;
        }
        return m2::make_unexpected("connect failed: " + m2::ToString(last_error));
    }
    return true;
}

expected<std::optional<TcpSocket>> TcpSocket::accept() {
    if (not IsServerSideListeningSocket()) {
        throw M2_ERROR("Accept called on a non-listening non-server socket");
    }

    sockaddr child_address{};
    int child_address_len = sizeof(child_address);
    SOCKET new_socket = ::accept(_platform_specific_data->socket, &child_address, &child_address_len);
    if (new_socket == INVALID_SOCKET) {
        if (WSAGetLastError() == WSAECONNRESET) {
            return std::nullopt;
        }
        return m2::make_unexpected("accept failed: " + m2::ToString(WSAGetLastError()));
    }

    TcpSocket child_socket;
    child_socket._clientAddr = reinterpret_cast<sockaddr_in*>(&child_address)->sin_addr.S_un.S_addr;
    child_socket._serverPort = _serverPort;
    child_socket._clientPort = reinterpret_cast<sockaddr_in*>(&child_address)->sin_port;
    child_socket._platform_specific_data = new detail::PlatformSpecificTcpSocketData{.socket = new_socket};
    return std::move(child_socket);
}

expected<int> TcpSocket::send(const uint8_t* buffer, size_t length) {
    if (not IsServerSideConnectedSocket() && not IsClientSideSocket()) {
        throw M2_ERROR("Send called on a non-connected socket");
    }

    auto send_result = ::send(_platform_specific_data->socket, reinterpret_cast<const char*>(buffer), I(length), 0);
    if (send_result == SOCKET_ERROR) {
        auto last_error = WSAGetLastError();
        if (last_error == WSAEWOULDBLOCK) {
            return -1;
        } else {
            return m2::make_unexpected("send failed: " + m2::ToString(last_error));
        }
    } else {
        return send_result;
    }
}

expected<int> TcpSocket::recv(uint8_t* buffer, size_t length) {
    if (not IsServerSideConnectedSocket() && not IsClientSideSocket()) {
        throw M2_ERROR("Recv called on a non-connected socket");
    }

    auto recv_result = ::recv(_platform_specific_data->socket, reinterpret_cast<char*>(buffer), I(length), 0);
    if (recv_result == SOCKET_ERROR) {
        auto last_error = WSAGetLastError();
        if (last_error == WSAEWOULDBLOCK) {
            return 0;
        }
        return m2::make_unexpected("recv failed: " + m2::ToString(last_error));
    } else if (recv_result == 0) {
        return m2::make_unexpected("Peer shut connection down");
    } else {
        return recv_result;
    }
}
