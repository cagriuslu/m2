#pragma once
#include "Message.h"
#include "SmallMessagePasser.h"
#include <m2/network/UdpSocket.h>
#include <queue>

namespace m2::multiplayer::lockstep {
	class MessagePasser {
		SmallMessagePasser _smallMessagePasser;
		std::queue<SmallMessageAndSender> _receivedSmallMessages;

	public:
		explicit MessagePasser(network::UdpSocket&& s) : _smallMessagePasser(std::move(s)) {}

		network::UdpSocket& GetSocket() { return _smallMessagePasser.GetSocket(); }

		enum class ReadResult {
			MESSAGE_RECEIVED,
		};
		expected<ReadResult> ReadMessages(std::queue<MessageAndSender>& out);

		[[nodiscard]] bool HasOutgoingData() const;

		enum class SendResult {
			MESSAGE_QUEUED,
		};
		expected<SendResult> SendMessage(MessageAndReceiver&& in);

		void Flush();
	};
}
