#pragma once
#include <cstdint>

namespace m2::network {
	using OrderNo = int32_t;
	using SequenceNo = int32_t;

	/// Class representing an IPv4 address
	class IpAddress final {
		uint32_t _inNetworkOrder{};

		explicit IpAddress(const uint32_t inNetworkOrder) : _inNetworkOrder(inNetworkOrder) {}

	public:
		IpAddress() = default;
		static IpAddress CreateFromString(const std::string&);
		static IpAddress CreateFromNetworkOrder(const uint32_t addr) { return IpAddress{addr}; }

		[[nodiscard]] uint32_t GetInNetworkOrder() const { return _inNetworkOrder; }
		[[nodiscard]] explicit operator bool() const { return _inNetworkOrder; }
		[[nodiscard]] bool operator==(const IpAddress& other) const { return _inNetworkOrder == other._inNetworkOrder; }
	};

	/// Class representing a port number
	class Port final {
		uint16_t _inNetworkOrder{};

		explicit Port(const uint16_t inNetworkOrder) : _inNetworkOrder(inNetworkOrder) {}

	public:
		Port() = default;
		static Port CreateFromHostOrder(uint16_t port);
		static Port CreateFromNetworkOrder(const uint16_t port) { return Port{port}; }

		[[nodiscard]] uint16_t GetInNetworkOrder() const { return _inNetworkOrder; }
		[[nodiscard]] explicit operator bool() const { return _inNetworkOrder; }
		[[nodiscard]] bool operator==(const Port& other) const { return _inNetworkOrder == other._inNetworkOrder; }
	};

	/// Group IP address and port together
	struct IpAddressAndPort {
		IpAddress ipAddress;
		Port port;

		[[nodiscard]] bool operator==(const IpAddressAndPort& other) const { return ipAddress == other.ipAddress && port == other.port; }
	};
}

namespace m2 {
	std::string ToString(const std::vector<network::OrderNo>&);
	std::string ToString(const network::IpAddress&);
	std::string ToString(const network::Port&);
	std::string ToString(const network::IpAddressAndPort&);
}
