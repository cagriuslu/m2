#include <m2/network/IpAddressAndPort.h>
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <netinet/in.h>
#endif

std::string m2::ToString(const network::IpAddressAndPort& ip_port) {
	const auto ip = ntohl(ip_port.first);
	const auto first = (ip & 0xFF000000u) >> 24;
	const auto second = (ip & 0xFF0000u) >> 16;
	const auto third = (ip & 0xFF00u) >> 8;
	const auto forth = (ip & 0xFFu) >> 0;
	const auto port = ntohs(ip_port.second);
	return m2::ToString(first) + "." + m2::ToString(second) + "." + m2::ToString(third) + "." + m2::ToString(forth) + ":" + m2::ToString(port);
}
