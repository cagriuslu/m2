#pragma once
#include <m2/multiplayer/lockstep//MessagePasser.h>
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
		const int _index;
		MessagePasser& _messagePasser;
		bool _readyState{}, _allPeersReachable{};
		std::optional<pb::LockstepFaultCode> _detectedFault{};
		std::list<Hash> _runningInputHash;

	public:
		static constexpr auto RunningInputHashCapacityInSeconds = 10;
		static inline auto RunningInputHashCapacity = m2g::LOCKSTEP_GAME_TICK_FREQUENCY * RunningInputHashCapacityInSeconds;

		ConnectionToClient(network::IpAddressAndPort address, int index, MessagePasser& messagePasser);

		// Accessors

		[[nodiscard]] const network::IpAddressAndPort& GetAddressAndPort() const { return _addressAndPort; }
		[[nodiscard]] bool IsAllOutgoingMessagesDelivered() const { return _messagePasser.GetConnectionStatistics(_addressAndPort)->IsAllOutgoingSmallMessagesDelivered(); }
		[[nodiscard]] bool GetReadyState() const { return _readyState; }
		[[nodiscard]] bool GetIfAllPeersReachable() const { return _allPeersReachable; }
		[[nodiscard]] std::optional<int32_t> GetInputHash(network::Timecode) const;
		[[nodiscard]] bool IsFaultDetected() const { return static_cast<bool>(_detectedFault); }

		// Modifiers

		void PublishPeerDetails(const pb::LockstepPeerDetails&);
		void SetReadyState(const bool state) { _readyState = state; }
		void MarkAsReachableToAllPeers() { _allPeersReachable = true; }
		void SetLobbyAsFrozen(const m2g::pb::LockstepGameInitParams&);
		void StoreRunningInputHash(const pb::LockstepPlayerInputs&);
		void SetFault(pb::LockstepFaultCode);
		void EndGame(const pb::LockstepGameEndReport&);

	private:
		void QueueOutgoingMessage(pb::LockstepMessage&& msg);
	};
}
