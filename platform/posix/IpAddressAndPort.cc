#include <m2/network/IpAddressAndPort.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

m2::expected<std::vector<std::string>> m2::network::get_ip_addresses() {
	ifaddrs *ifap;
	if (getifaddrs(&ifap) == -1) {
		return m2::make_unexpected(strerror(errno));
	}

	// Gather IP addresses
	std::vector<std::string> ip_addresses;
	for (ifaddrs* iface = ifap; iface; iface = iface->ifa_next) {
		if (iface->ifa_addr && iface->ifa_addr->sa_family == AF_INET) {
			if (auto address = inet_ntoa(reinterpret_cast<sockaddr_in*>(iface->ifa_addr)->sin_addr)) {
				ip_addresses.emplace_back(address);
			}
		}
	}

	freeifaddrs(ifap);
	return ip_addresses;
}
