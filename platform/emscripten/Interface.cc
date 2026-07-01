#include <m2/network/Interface.h>

m2::expected<std::vector<m2::network::IpAddress>> m2::network::GetInterfaces() {
	// Dummy localhost interface
	return std::vector<IpAddress>{IpAddress::CreateLocalhost()};
}
