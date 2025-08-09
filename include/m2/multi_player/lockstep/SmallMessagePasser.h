#pragma once
#include "Message.h"
#include <m2/network/UdpSocket.h>
#include <m2/network/Types.h>
#include <m2/multi_player/lockstep/ConnectionStatistics.h>
#include <m2/Chrono.h>
#include <Lockstep.pb.h>
#include <map>
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
			bool sendAck{};

			// Sending parameters

			network::OrderNo nextOutgoingOrderNo{1};
			/// Front of the queue corresponds to the oldest non-acknowledged message
			std::deque<OutgoingNackSmallMessage> outgoingNackMessages;
			std::optional<Stopwatch> lastMessageSentAt;

			// Receiving parameters

			/// Order number of the last message returned via ReadSmallMessages.
			network::OrderNo lastOrderlyReceivedOrderNo{0};
			/// Messages received after a gap. These won't be returned until the gap is closed. Front is the oldest.
			std::map<network::OrderNo, pb::LockstepSmallMessage> messagesSinceGap;

			explicit PeerConnectionParameters(const network::IpAddressAndPort address) : peerAddress(address) {}

			[[nodiscard]] pb::LockstepUdpPacket CreateOutgoingPacketFromTailMessages() const;
			[[nodiscard]] int32_t GetMostRecentAck() const;
			[[nodiscard]] int32_t GetAckHistoryBits() const;
			[[nodiscard]] int32_t GetOldestNack() const;

			void ProcessPeerAcks(int32_t mostRecentAck, int32_t ackHistoryBits, int32_t oldestNack);
			void ProcessReceivedMessages(google::protobuf::RepeatedPtrField<pb::LockstepSmallMessage>*, std::queue<SmallMessageAndSender>& out);
		};

		network::UdpSocket _socket;
		bool _blockUnknownConnections;
		char _recvBuffer[1520] = {};
		std::vector<PeerConnectionParameters> _peerConnectionParameters;

	public:
		explicit SmallMessagePasser(network::UdpSocket&& s) : _socket(std::move(s)), _blockUnknownConnections(_socket.IsClientSideSocket()) {}

		// Accessors

		[[nodiscard]] network::UdpSocket& GetSocket() { return _socket; }
		[[nodiscard]] ConnectionStatistics* GetConnectionStatistics(const network::IpAddressAndPort& address);

		// Modifiers

		void BlockUnknownConnections() { _blockUnknownConnections = true; }
		void ReadSmallMessages(std::queue<SmallMessageAndSender>& out);
		void_expected SendSmallMessage(SmallMessageAndReceiver&& in);
		void SendRetransmissionsAndAcks();

	private:
		PeerConnectionParameters* FindPeerConnectionParameters(const network::IpAddressAndPort& address);
		PeerConnectionParameters& FindOrCreatePeerConnectionParameters(const network::IpAddressAndPort& address);
	};
}
