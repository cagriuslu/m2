#include <m2/network/Interface.h>

using namespace m2;
using namespace m2::network;

std::vector<IpAddress> network::InferLanAddresses(const std::vector<IpAddress>& ipAddresses) {
	std::vector<IpAddress> lanAddresses;
	for (const auto& ip : ipAddresses) {
		const auto firstOctet = (ip.GetInNetworkOrder()) & 0xFF;
		const auto secondOctet = (ip.GetInNetworkOrder() >> 8) & 0xFF;
		const auto thirdOctet = (ip.GetInNetworkOrder() >> 16) & 0xFF;
		const auto forthOctet = (ip.GetInNetworkOrder() >> 24) & 0xFF;
		if ((firstOctet == 127 && secondOctet == 0 && thirdOctet == 0 && forthOctet == 1)
				|| firstOctet == 10
				|| (firstOctet == 172 && 16 <= secondOctet && secondOctet < 32)
				|| (firstOctet == 192 && secondOctet == 168)
				|| (firstOctet == 169 && secondOctet == 254)
				|| (firstOctet == 100 && secondOctet == 64)) {
			lanAddresses.emplace_back(ip);
		}
	}
	return lanAddresses;
}
