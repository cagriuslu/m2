#pragma once
#include "Message.h"
#include <m2/network/UdpSocket.h>
#include <m2/network/Types.h>
#include <m2/Chrono.h>
#include <Lockstep.pb.h>
#include <deque>

namespace m2::multiplayer::lockstep {
	class SmallMessagePasser {
		struct PeerConnectionParameters {
			network::IpAddressAndPort peerAddress;

			// Sending parameters

			network::OrderNo nextOutgoingOrderNo{1};
			/// Front of the queue corresponds to the oldest non-acknowledged message
			std::deque<std::pair<network::OrderNo,pb::LockstepSmallMessage>> outgoingNackMessages;
			std::optional<Stopwatch> lastMessageSentAt;

			pb::LockstepUdpPacket CreateOutgoingPacketFromTailMessages() const;

			// Receiving parameters

			/// Order number of the last message returned to the user of this class via ReadSmallMessages. If there is a
			/// gap, oldestGapOrderNo must be one more than lastOrderlyReceivedOrderNo.
			network::OrderNo lastOrderlyReceivedOrderNo{0};
			struct GapHistorySinceOldestNack {
				network::OrderNo oldestGapOrderNo;
				/// Front of the queue corresponds to oldestNackOrderNo, thus it is nullopt. Back of the queue
				/// corresponds to the most recently received message, thus it is NOT nullopt.
				std::deque<std::optional<pb::LockstepSmallMessage>> messagesSinceOldestGap;
			};
			std::optional<GapHistorySinceOldestNack> gapHistory; /// Exists only if there's a gap

			[[nodiscard]] int32_t GetMostRecentAck() const;
			[[nodiscard]] int32_t GetAckHistoryBits() const;
			int32_t GetOldestNack() const;
		};

		network::UdpSocket _socket;
		char _recvBuffer[1520] = {};
		std::vector<PeerConnectionParameters> _peerConnectionParameters;

	public:
		explicit SmallMessagePasser(network::UdpSocket&& s) : _socket(std::move(s)) {}

		// Accessors

		[[nodiscard]] network::UdpSocket& GetSocket() { return _socket; }

		// Modifiers

		void ReadSmallMessages(std::queue<SmallMessageAndSender>& out);

		void SendSmallMessage(SmallMessageAndReceiver&& in);

	private:
		PeerConnectionParameters* FindPeerConnectionParameters(const network::IpAddressAndPort& address);
		PeerConnectionParameters& FindOrCreatePeerConnectionParameters(const network::IpAddressAndPort& address);
	};
}
