#pragma once
#include "Message.h"
#include <m2/network/UdpSocket.h>
#include <m2/network/Types.h>
#include <m2/multi_player/lockstep/ConnectionStatistics.h>
#include <m2/Chrono.h>
#include <Lockstep.pb.h>
#include <deque>

namespace m2::multiplayer::lockstep {
	/// \brief Small lockstep UDP message passer
	/// \details This class is responsible for passing LockstepSmallMessage protobuf objects between the instances of
	/// itself belonging to different peers. Small messages are small enough to be carried by one UDP packet. If they
	/// are small enough, more than one such message can even be carried inside one UDP packet. This class ensures that
	/// every small message is delivered to the peer by means of ACKs and retransmissions. The messages are then
	/// reordered on the receiver side.
	class SmallMessagePasser {
		/// \details Each peer, if ever responded to by this instance, gains an entry in the list of connections.
		struct PeerConnectionParameters {
			network::IpAddressAndPort peerAddress;
			ConnectionStatistics connectionStatistics;

			// Sending parameters

			network::OrderNo nextOutgoingOrderNo{1};
			/// Front of the queue corresponds to the oldest non-acknowledged message
			std::deque<OutgoingNackSmallMessage> outgoingNackMessages;
			std::optional<Stopwatch> lastMessageSentAt;

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

			explicit PeerConnectionParameters(const network::IpAddressAndPort address) : peerAddress(address), connectionStatistics(nextOutgoingOrderNo) {}

			[[nodiscard]] pb::LockstepUdpPacket CreateOutgoingPacketFromTailMessages() const;
			[[nodiscard]] int32_t GetMostRecentAck() const;
			[[nodiscard]] int32_t GetAckHistoryBits() const;
			[[nodiscard]] int32_t GetOldestNack() const;
		};

		network::UdpSocket _socket;
		char _recvBuffer[1520] = {};
		std::vector<PeerConnectionParameters> _peerConnectionParameters;

	public:
		explicit SmallMessagePasser(network::UdpSocket&& s) : _socket(std::move(s)) {}

		// Accessors

		[[nodiscard]] network::UdpSocket& GetSocket() { return _socket; }
		[[nodiscard]] ConnectionStatistics* GetConnectionStatistics(const network::IpAddressAndPort& address);

		// Modifiers

		void ReadSmallMessages(std::queue<SmallMessageAndSender>& out);

		void_expected SendSmallMessage(SmallMessageAndReceiver&& in);
		void SendRetransmissions();

	private:
		PeerConnectionParameters* FindPeerConnectionParameters(const network::IpAddressAndPort& address);
		PeerConnectionParameters& FindOrCreatePeerConnectionParameters(const network::IpAddressAndPort& address);
	};
}
