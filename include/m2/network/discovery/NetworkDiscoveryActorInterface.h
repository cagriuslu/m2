#pragma once
#include "NetworkDiscoveryActor.h"
#include <m2/mt/actor/ActorInterfaceBase.h>

namespace m2::network::discovery {
	class NetworkDiscoveryActorInterface final : public ActorInterfaceBase<NetworkDiscoveryActor, NetworkDiscoveryActorInput, NetworkDiscoveryActorOutput> {
		std::vector<IpAddressAndPort> _lastDiscoveredPeers;

	public:
		explicit NetworkDiscoveryActorInterface(NetworkDiscoveryActor::Role role) : ActorInterfaceBase(std::move(role)) {}

		/// A previously discovered peer is never removed from the list. New peers are appended to the list and the
		/// order of theexisting elements are retained.
		[[nodiscard]] decltype(_lastDiscoveredPeers) GetDiscoveredPeers();
	};
}
