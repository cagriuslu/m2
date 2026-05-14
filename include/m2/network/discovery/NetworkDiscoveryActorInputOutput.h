#pragma once
#include <m2/network/Types.h>

namespace m2::network::discovery {
	struct NetworkDiscoveryActorInput {};
	struct NetworkDiscoveryActorOutput {
		std::vector<IpAddressAndPort> discoveredPeers;
	};
}
