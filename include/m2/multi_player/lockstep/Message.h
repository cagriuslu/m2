#pragma once
#include <m2/network/IpAddressAndPort.h>
#include <Lockstep.pb.h>
#include <utility>

namespace m2::multiplayer::lockstep {
	constexpr int MAX_UDP_PACKET_SIZE = 1200;
	int LockstepUdpPacketHeaderSize();
	constexpr int EACH_SMALL_MESSAGE_HEADER_SIZE = 4; // 1 byte higher than necessary
	int LockstepSmallMessageMaxSize();

	struct MessageAndSender {
		pb::LockstepMessage message;
		network::IpAddressAndPort sender;
	};

	struct MessageAndReceiver {
		pb::LockstepMessage message;
		network::IpAddressAndPort receiver;
	};

	struct SmallMessageAndSender {
		pb::LockstepSmallMessage smallMessage;
		network::IpAddressAndPort sender;
	};

	struct SmallMessageAndReceiver {
		pb::LockstepSmallMessage smallMessage;
		network::IpAddressAndPort receiver;
	};
}
