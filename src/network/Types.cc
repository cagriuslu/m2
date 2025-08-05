#include <m2/network/Types.h>
#include <m2/M2.h>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

m2::network::IpAddress m2::network::IpAddress::CreateFromString(const std::string& s) {
	return IpAddress{inet_addr(s.c_str())};
}

m2::network::Port m2::network::Port::CreateFromHostOrder(uint16_t port) {
	return Port{htons(port)};
}

std::string m2::ToString(const network::IpAddress& ip) {
	const auto inNetworkOrder = ip.GetInNetworkOrder();
	const auto first = (inNetworkOrder & 0xFFu) >> 0;
	const auto second = (inNetworkOrder & 0xFF00u) >> 8;
	const auto third = (inNetworkOrder & 0xFF0000u) >> 16;
	const auto forth = (inNetworkOrder & 0xFF000000u) >> 24;
	return ToString(first) + "." + ToString(second) + "." + ToString(third) + "." + ToString(forth);
}
std::string m2::ToString(const network::Port& port) {
	const auto inNetworkOrder = port.GetInNetworkOrder();
	const auto inHostOrder = ntohs(inNetworkOrder);
	return ToString(inHostOrder);
}
std::string m2::ToString(const network::IpAddressAndPort& addrAndPort) {
	return ToString(addrAndPort.ipAddress) + ":" + ToString(addrAndPort.port);
}
