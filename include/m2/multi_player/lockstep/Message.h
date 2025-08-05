#pragma once
#include <m2/network/Types.h>
#include <Lockstep.pb.h>
#include <utility>

namespace m2::multiplayer::lockstep {
	constexpr int MAX_UDP_PACKET_SIZE = 1200;
	/// Size of the fields of a UDP packet except the payload (small messages)
	int UdpPacketHeaderSize();
	/// \brief How many bytes is added to the UDP packet for each new small message
	/// \details In reality, this number is either 2 or 3, depending on the size of the small message.
	constexpr int N_BYTES_ADDED_TO_HEADER_FOR_EACH_SMALL_MESSAGE = 4;
	/// Largest allowed small message size that would still fit into the UDP packet
	int SmallMessageMaxSize();
	/// Size of the fields of a small message except the payload (complete message or message part)
	int SmallMessageHeaderSize();
	/// \brief Size of the maximum allowed complete message that can be carried inside the small message
	/// \details Also corresponds to maximum message part size
	int CompleteMessageMaxSize();

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
