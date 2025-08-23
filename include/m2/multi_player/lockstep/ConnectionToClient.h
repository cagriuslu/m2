#pragma once
#include <m2/multi_player/lockstep//MessagePasser.h>
#include <m2/network/Types.h>

namespace m2::multiplayer::lockstep {
	class ConnectionToClient final {
		const network::IpAddressAndPort _addressAndPort;
		MessagePasser& _messagePasser;
		bool _readyState{};

	public:
		ConnectionToClient(network::IpAddressAndPort address, MessagePasser& messagePasser);

		// Accessors

		const network::IpAddressAndPort& GetAddressAndPort() const { return _addressAndPort; }
		bool GetReadyState() const { return _readyState; }

		// Modifiers

		void SetReadyState(const bool state) { _readyState = state; }
		void SetLobbyAsFrozen(const m2g::pb::LockstepGameInitParams&);

	private:
		void QueueOutgoingMessage(pb::LockstepMessage&& msg);
	};
}
