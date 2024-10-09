#include <m2/network/Detail.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

std::string m2::to_string(const std::pair<in_addr_t,in_port_t>& address_and_port) {
	auto first = (static_cast<uint32_t>(address_and_port.first) & 0xFF000000u ) >> 24;
	auto second = (static_cast<uint32_t>(address_and_port.first) & 0xFF0000u) >> 16;
	auto third = (static_cast<uint32_t>(address_and_port.first) & 0xFF00u) >> 8;
	auto forth = (static_cast<uint32_t>(address_and_port.first) & 0xFFu) >> 0;
	auto port = static_cast<uint32_t>(address_and_port.second);
	return std::to_string(first) + "." + std::to_string(second) + "." + std::to_string(third) + "." + std::to_string(forth) + ":" + std::to_string(port);
}

m2::expected<int> m2::network::select(int max_fd, fd_set* read, fd_set* write, uint64_t timeout_ms) {
	timeval tv{};
	tv.tv_sec = static_cast<int64_t>(timeout_ms) / 1000;
	tv.tv_usec = static_cast<int32_t>((timeout_ms % 1000) * 1000);

	int select_result = ::select(max_fd + 1, read, write, nullptr, &tv);
	if (select_result == -1) {
		return make_unexpected(strerror(errno));
	} else {
		return select_result;
	}
}

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
