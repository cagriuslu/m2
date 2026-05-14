#include <m2/network/discovery/NetworkDiscoveryActorInterface.h>

using namespace m2;
using namespace m2::network;
using namespace m2::network::discovery;

decltype(NetworkDiscoveryActorInterface::_lastDiscoveredPeers) NetworkDiscoveryActorInterface::GetDiscoveredPeers() {
	// Pop all messages from the queue, only read the last one
	std::optional<NetworkDiscoveryActorOutput> output, last;
	while (GetActorOutbox().TryPopMessage(output)) {
		last = std::move(output);
	}
	if (last) {
		_lastDiscoveredPeers = std::move(last->discoveredPeers);
	}
	return _lastDiscoveredPeers;
}
