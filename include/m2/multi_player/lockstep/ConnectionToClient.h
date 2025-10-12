#pragma once
#include <m2/multi_player/lockstep//MessagePasser.h>
#include <m2/network/Types.h>
#include <m2/ProxyHelper.h>
#include <list>

namespace m2::multiplayer::lockstep {
	class ConnectionToClient final {
		struct Hash {
			network::Timecode timecode{};
			int32_t hash{};
		};

		const network::IpAddressAndPort _addressAndPort;
		MessagePasser& _messagePasser;
		bool _readyState{};
		std::list<Hash> _runningInputHash;

	public:
		static constexpr auto RunningInputHashCapacityInSeconds = 10;
		static inline auto RunningInputHashCapacity = I(std::ceilf(m2g::LockstepGameTickFrequencyF)) * RunningInputHashCapacityInSeconds;

		ConnectionToClient(network::IpAddressAndPort address, MessagePasser& messagePasser);

		// Accessors

		[[nodiscard]] const network::IpAddressAndPort& GetAddressAndPort() const { return _addressAndPort; }
		[[nodiscard]] bool IsAllOutgoingMessagesDelivered() const { return _messagePasser.GetConnectionStatistics(_addressAndPort)->IsAllOutgoingSmallMessagesDelivered(); }
		[[nodiscard]] bool GetReadyState() const { return _readyState; }

		// Modifiers

		void PublishPeerDetails(const pb::LockstepPeerDetails&);
		void SetReadyState(const bool state) { _readyState = state; }
		void SetLobbyAsFrozen(const m2g::pb::LockstepGameInitParams&);
		void StoreRunningInputHash(const pb::LockstepPlayerInputs&);

	private:
		void QueueOutgoingMessage(pb::LockstepMessage&& msg);
	};
}
