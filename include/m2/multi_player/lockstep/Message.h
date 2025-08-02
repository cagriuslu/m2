#pragma once
#include <m2/network/IpAddressAndPort.h>
#include <Lockstep.pb.h>
#include <utility>

namespace m2::multiplayer::lockstep {
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
		int32_t orderNo;
	};

	struct SmallMessageAndReceiver {
		pb::LockstepSmallMessage smallMessage;
		network::IpAddressAndPort receiver;
		int32_t orderNo;
	};
}
