#pragma once
#include "NetworkDiscoveryActorInputOutput.h"
#include <m2/mt/actor/ActorBase.h>
#include <m2/network/UdpSocket.h>

namespace m2::network::discovery {
	class NetworkDiscoveryActor final : ActorBase<NetworkDiscoveryActorInput, NetworkDiscoveryActorOutput> {
	public:
		struct SpeakerParameters {
			IpAddressAndPort multicastAddress;
			IpAddress multicastInterface;
			int32_t gameHash;
			Port gamePort;
		};
		struct ListenerParameters {
			IpAddressAndPort multicastAddress;
			int32_t gameHash;
		};
		using Role = std::variant<SpeakerParameters, ListenerParameters>;

	private:
		Role _role;
		std::optional<UdpSocket> _socket;
		Stopwatch _lastHeartbeat{};
		char _recvBuffer[256] = {};
		std::vector<IpAddressAndPort> _knownSpeakers;

	public:
		explicit NetworkDiscoveryActor(Role role) : ActorBase(), _role(std::move(role)) {}
		~NetworkDiscoveryActor() override = default;

		[[nodiscard]] const char* ThreadNameForLogging() const override { return "ND"; }

		bool Initialize(MessageBox<NetworkDiscoveryActorInput>&, MessageBox<NetworkDiscoveryActorOutput>&) override;

		bool operator()(MessageBox<NetworkDiscoveryActorInput>&, MessageBox<NetworkDiscoveryActorOutput>&) override;

		void Deinitialize(MessageBox<NetworkDiscoveryActorInput>&, MessageBox<NetworkDiscoveryActorOutput>&) override {}
	};
}
