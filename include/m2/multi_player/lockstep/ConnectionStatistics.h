#pragma once

namespace m2::multiplayer::lockstep {
	class ConnectionStatistics {
		int _totalQueuedOutgoingPackets{};
		int _totalAckedOutgoingPackets{};

	public:
		// Accessors

		[[nodiscard]] int GetTotalQueuedOutgoingPackets() const { return _totalQueuedOutgoingPackets; }
		[[nodiscard]] int GetTotalAckedOutgoingPackets() const { return _totalAckedOutgoingPackets; }

		// Modifiers

		void IncrementOutgoingPacketCount(const int count = 1) { _totalQueuedOutgoingPackets += count; }
		void IncrementAckedOutgoingPacketCount(const int count = 1) { _totalAckedOutgoingPackets += count; }
	};
}
