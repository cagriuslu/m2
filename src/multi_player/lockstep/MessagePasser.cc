#include <m2/multi_player/lockstep/MessagePasser.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

expected<MessagePasser::ReadResult> MessagePasser::ReadMessages(std::queue<MessageAndSender>& out) {
	_smallMessagePasser.ReadSmallMessages(_receivedSmallMessages);

	// TODO assuming small message is complete

	while (not _receivedSmallMessages.empty()) {
		auto& smallMsg = _receivedSmallMessages.front();
		m2SucceedOrThrowMessage(smallMsg.smallMessage.has_complete_message(), "Message doesn't have a complete message"); // TODO temporary
		out.push(MessageAndSender{
			.message = std::move(*smallMsg.smallMessage.release_complete_message()),
			.sender = smallMsg.sender
		});
		_receivedSmallMessages.pop();
	}

	return ReadResult::MESSAGE_RECEIVED;
}

expected<MessagePasser::SendResult> MessagePasser::SendMessage(MessageAndReceiver&& in) {
	// TODO assuming complete message
	pb::LockstepSmallMessage smallMsg;
	// TODO sequence no
	smallMsg.mutable_complete_message()->CopyFrom(in.message);
	// TODO reconstructed message size
	_smallMessagePasser.SendSmallMessage(SmallMessageAndReceiver{
		.smallMessage = std::move(smallMsg),
		.receiver = std::move(in.receiver)
	});
	return SendResult::MESSAGE_QUEUED;
}
