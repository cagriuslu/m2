#include <m2/network/IpAddressAndPort.h>

std::string m2::to_string(const network::IpAddressAndPort& ip_port) {
	auto first = (static_cast<uint32_t>(ip_port.first) & 0xFF000000u ) >> 24;
	auto second = (static_cast<uint32_t>(ip_port.first) & 0xFF0000u) >> 16;
	auto third = (static_cast<uint32_t>(ip_port.first) & 0xFF00u) >> 8;
	auto forth = (static_cast<uint32_t>(ip_port.first) & 0xFFu) >> 0;
	auto port = static_cast<uint32_t>(ip_port.second);
	return std::to_string(first) + "." + std::to_string(second) + "." + std::to_string(third) + "." + std::to_string(forth) + ":" + std::to_string(port);
}
