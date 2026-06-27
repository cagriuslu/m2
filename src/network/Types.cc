#include <m2/network/Types.h>
#include <m2/M2.h>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

m2::network::IpAddress m2::network::IpAddress::CreateLocalhost() {
	return CreateFromString("127.0.0.1");
}
m2::network::IpAddress m2::network::IpAddress::CreateFromString(const std::string& s) {
	return IpAddress{inet_addr(s.c_str())};
}

m2::network::Port m2::network::Port::CreateFromHostOrder(uint16_t port) {
	return Port{htons(port)};
}

auto std::formatter<m2::network::IpAddress>::format(const m2::network::IpAddress& ip, std::format_context& ctx) const -> std::format_context::iterator {
	const auto inNetworkOrder = ip.GetInNetworkOrder();
	const auto first = (inNetworkOrder & 0xFFu) >> 0;
	const auto second = (inNetworkOrder & 0xFF00u) >> 8;
	const auto third = (inNetworkOrder & 0xFF0000u) >> 16;
	const auto forth = (inNetworkOrder & 0xFF000000u) >> 24;
	return std::formatter<std::string>::format(std::format("{}.{}.{}.{}", first, second, third, forth), ctx);
}
auto std::formatter<m2::network::Port>::format(const m2::network::Port& port, std::format_context& ctx) const -> std::format_context::iterator {
	const auto inNetworkOrder = port.GetInNetworkOrder();
	const auto inHostOrder = ntohs(inNetworkOrder);
	return std::formatter<std::string>::format(std::format("{}", inHostOrder), ctx);
}
auto std::formatter<m2::network::IpAddressAndPort>::format(const m2::network::IpAddressAndPort& addrAndPort, std::format_context& ctx) const -> std::format_context::iterator {
	return std::formatter<std::string>::format(
		std::format("{}:{}", addrAndPort.ipAddress, addrAndPort.port), ctx);
}
