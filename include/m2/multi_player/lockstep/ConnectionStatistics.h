#pragma once

namespace m2::multiplayer::lockstep {
	class ConnectionStatistics {
		const network::OrderNo& _nextOutgoingOrderNo;

	public:
		explicit ConnectionStatistics(const network::OrderNo& nextOutgoingOrderNo) : _nextOutgoingOrderNo(nextOutgoingOrderNo) {}

		[[nodiscard]] int GetQueuedMessageCount() const { return _nextOutgoingOrderNo - 1; }
		[[nodiscard]] int GetAcknowledgedOutgoingMessageCount() const { return 0; } // TODO
	};
}
