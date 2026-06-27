#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace m2::network {
	using OrderNo = uint32_t;
	using SequenceNo = uint32_t;
	using Timecode = uint32_t;

	/// Class representing an IPv4 address
	class IpAddress final {
		uint32_t _inNetworkOrder{};

		explicit IpAddress(const uint32_t inNetworkOrder) : _inNetworkOrder(inNetworkOrder) {}

	public:
		IpAddress() = default;
		static IpAddress CreateLocalhost();
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


#include <format>
template <> struct std::formatter<m2::network::IpAddress> : std::formatter<std::string> {
	auto format(const m2::network::IpAddress& ip, std::format_context& ctx) const -> std::format_context::iterator;
};
template <> struct std::formatter<m2::network::Port> : std::formatter<std::string> {
	auto format(const m2::network::Port& port, std::format_context& ctx) const -> std::format_context::iterator;
};
template <> struct std::formatter<m2::network::IpAddressAndPort> : std::formatter<std::string> {
	auto format(const m2::network::IpAddressAndPort& addrAndPort, std::format_context& ctx) const -> std::format_context::iterator;
};
