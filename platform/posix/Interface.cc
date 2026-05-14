#include <m2/network/Interface.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

m2::expected<std::vector<m2::network::IpAddress>> m2::network::GetInterfaces() {
	ifaddrs *ifap;
	if (getifaddrs(&ifap) == -1) {
		return m2::make_unexpected(strerror(errno));
	}
	std::vector<IpAddress> interfaces;
	for (const ifaddrs* iface = ifap; iface; iface = iface->ifa_next) {
		if (iface->ifa_addr && iface->ifa_addr->sa_family == AF_INET) {
			interfaces.emplace_back(IpAddress::CreateFromNetworkOrder(reinterpret_cast<sockaddr_in*>(iface->ifa_addr)->sin_addr.s_addr));
		}
	}
	freeifaddrs(ifap);
	return interfaces;
}
