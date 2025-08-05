#pragma once
#include <m2/network/Types.h>
#include <m2/Meta.h>

namespace m2::network {
	// Forward declarations
	struct Select;
	namespace detail {
		struct PlatformSpecificSocketData;
	}

	class UdpSocket {
		detail::PlatformSpecificSocketData* _platformSpecificData{};
		Port _selfPort{};

		/// \details Platform specific data should be initialized after calling this constructor
		UdpSocket() = default;

	public:
		static expected<UdpSocket> CreateServerSideSocket(const Port& port);
		static expected<UdpSocket> CreateClientSideSocket();
		UdpSocket(const UdpSocket& other) = delete;
		UdpSocket& operator=(const UdpSocket& other) = delete;
		UdpSocket(UdpSocket&& other) noexcept;
		UdpSocket& operator=(UdpSocket&& other) noexcept;
		~UdpSocket();

		// Accessors

		[[nodiscard]] bool IsServerSideSocket() const { return static_cast<bool>(_selfPort); }
		[[nodiscard]] bool IsClientSideSocket() const { return not _selfPort; }

		// Modifiers

		/// If expected, returns the number of bytes queued into kernel's buffer. If the buffer is full, the return
		/// value may be zero, or less than `length`. Otherwise, unexpected is returned with the error message.
		void_expected send(const IpAddressAndPort& peerAddrAndPort, const uint8_t* buffer, size_t length);
		void_expected send(const IpAddressAndPort& peerAddrAndPort, const char* buffer, const size_t length) { return send(peerAddrAndPort, reinterpret_cast<const uint8_t*>(buffer), length); }

		/// If expected, returns the number of bytes actually received. Otherwise, unexpected is returned with the error
		/// message.
		expected<std::pair<int, IpAddressAndPort>> recv(uint8_t* buffer, size_t length);
		expected<std::pair<int, IpAddressAndPort>> recv(char* buffer, const size_t length) { return recv(reinterpret_cast<uint8_t*>(buffer), length); }

		/// Give Select class access to the internals
		friend Select;
	};
}
