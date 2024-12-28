#pragma once
#include <m2/network/IpAddressAndPort.h>
#include <m2/Meta.h>

namespace m2 {
	// Forward declarations
	struct Select;
	namespace detail {
		struct PlatformSpecificUdpSocketData;
	}

	class UdpSocket {
		detail::PlatformSpecificUdpSocketData* _platformSpecificData{};

		/// Platform specific data should be initialized after calling this constructor
		UdpSocket() = default;

	public:
		static expected<UdpSocket> Create(network::Port localPort);
		UdpSocket(const UdpSocket& other) = delete;
		UdpSocket& operator=(const UdpSocket& other) = delete;
		UdpSocket(UdpSocket&& other) noexcept;
		UdpSocket& operator=(UdpSocket&& other) noexcept;
		~UdpSocket();

		expected<int> send(network::IpAddress peerAddr, network::Port peerPort, const uint8_t* buffer, size_t length);
		expected<int> send(const network::IpAddress peerAddr, const network::Port peerPort, const char* buffer, const size_t length) { return send(peerAddr, peerPort, reinterpret_cast<const uint8_t*>(buffer), length); }

		expected<std::pair<int, network::IpAddressAndPort>> recv(uint8_t* buffer, size_t length);
		expected<std::pair<int, network::IpAddressAndPort>> recv(char* buffer, const size_t length) { return recv(reinterpret_cast<uint8_t*>(buffer), length); }
	};
}
