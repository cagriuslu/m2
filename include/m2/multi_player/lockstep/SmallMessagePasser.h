#pragma once
#include "Message.h"
#include <m2/network/UdpSocket.h>
#include <Lockstep.pb.h>
#include <deque>

namespace m2::multiplayer::lockstep {
	class SmallMessagePasser {
		network::UdpSocket _socket;
		char _recvBuffer[1520] = {};

	public:
		explicit SmallMessagePasser(network::UdpSocket&& s) : _socket(std::move(s)) {}

		// Accessors

		[[nodiscard]] network::UdpSocket& GetSocket() { return _socket; }

		// Modifiers

		void ReadSmallMessages(std::queue<SmallMessageAndSender>& out);

		void SendSmallMessage(SmallMessageAndReceiver&& in);
	};
}
