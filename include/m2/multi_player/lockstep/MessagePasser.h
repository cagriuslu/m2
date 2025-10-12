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
			network::SequenceNo nextOutgoingSequenceNo{1};
			network::SequenceNo lastReceivedSequenceNo{0};
		};

		SmallMessagePasser _smallMessagePasser;
		std::vector<PeerConnectionParameters> _peerConnectionParameters;
		std::queue<SmallMessageAndSender> _receivedSmallMessages;

	public:
		explicit MessagePasser(network::UdpSocket&& s) : _smallMessagePasser(std::move(s)) {}

		// Accessors

		network::UdpSocket& GetSocket() { return _smallMessagePasser.GetSocket(); }
		/// Returns the connection statistics with the peer whose address is given. Returns null if no messages have
		/// been sent to or received from the peer.
		const ConnectionStatistics* GetConnectionStatistics(const network::IpAddressAndPort& address) const { return _smallMessagePasser.GetConnectionStatistics(address); }

		// Modifiers

		void_expected ReadMessages(std::queue<MessageAndSender>& out);
		void_expected QueueMessage(MessageAndReceiver&& in);
		void_expected SendOutgoingPackets() { return _smallMessagePasser.SendOutgoingPackets(); }
		void Flush() { _smallMessagePasser.Flush(); }

	private:
		PeerConnectionParameters* FindPeerConnectionParameters(const network::IpAddressAndPort& address);
		PeerConnectionParameters& FindOrCreatePeerConnectionParameters(const network::IpAddressAndPort& address);
	};
}
