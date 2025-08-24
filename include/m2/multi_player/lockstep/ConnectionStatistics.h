#pragma once

namespace m2::multiplayer::lockstep {
	class ConnectionStatistics {
		int _totalQueuedOutgoingSmallMessages{};
		int _totalAckedOutgoingSmallMessages{};

	public:
		// Accessors

		[[nodiscard]] int GetTotalQueuedOutgoingSmallMessages() const { return _totalQueuedOutgoingSmallMessages; }
		[[nodiscard]] int GetTotalAckedOutgoingSmallMessages() const { return _totalAckedOutgoingSmallMessages; }
		[[nodiscard]] bool IsAllOutgoingSmallMessagesDelivered() const { return _totalQueuedOutgoingSmallMessages == _totalAckedOutgoingSmallMessages; }

		// Modifiers

		void IncrementOutgoingSmallMessageCount(const int count = 1) { _totalQueuedOutgoingSmallMessages += count; }
		void IncrementAckedOutgoingSmallMessageCount(const int count = 1) { _totalAckedOutgoingSmallMessages += count; }
	};
}
