#pragma once
#include "Message.h"
#include "SmallMessagePasser.h"
#include "ConnectionStatistics.h"
#include <m2/network/UdpSocket.h>
#include <queue>

namespace m2::multiplayer::lockstep {
	class MessagePasser {
		struct PeerConnectionParameters {
			network::IpAddressAndPort peerAddress;

			// Sending parameters

			network::SequenceNo nextOutgoingSequenceNo{1};

			// Receiving parameters

			network::SequenceNo lastReceivedSequenceNo{0};
		};

		SmallMessagePasser _smallMessagePasser;
		std::vector<PeerConnectionParameters> _peerConnectionParameters;
		std::queue<SmallMessageAndSender> _receivedSmallMessages;

	public:
		explicit MessagePasser(network::UdpSocket&& s) : _smallMessagePasser(std::move(s)) {}

		network::UdpSocket& GetSocket() { return _smallMessagePasser.GetSocket(); }
		ConnectionStatistics* GetConnectionStatistics(const network::IpAddressAndPort& address) { return _smallMessagePasser.GetConnectionStatistics(address); }

		enum class ReadResult {
			MESSAGE_RECEIVED,
		};
		expected<ReadResult> ReadMessages(std::queue<MessageAndSender>& out);

		enum class SendResult {
			MESSAGE_QUEUED,
		};
		expected<SendResult> SendMessage(MessageAndReceiver&& in);

		void Flush();

	private:
		PeerConnectionParameters* FindPeerConnectionParameters(const network::IpAddressAndPort& address);
		PeerConnectionParameters& FindOrCreatePeerConnectionParameters(const network::IpAddressAndPort& address);
	};
}
