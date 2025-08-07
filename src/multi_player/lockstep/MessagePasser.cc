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
	if (CompleteMessageMaxSize() < I(in.message.ByteSizeLong())) {
		return make_unexpected("Splitting large message into smaller chunks is not yet implemented");
	}
	auto& peerConnParams = FindOrCreatePeerConnectionParameters(in.receiver);
	pb::LockstepSmallMessage smallMsg;
	smallMsg.set_message_sequence_no(peerConnParams.nextOutgoingSequenceNo++);
	smallMsg.mutable_complete_message()->Swap(&in.message);
	_smallMessagePasser.SendSmallMessage(SmallMessageAndReceiver{
		.smallMessage = std::move(smallMsg),
		.receiver = in.receiver
	});
	return SendResult::MESSAGE_QUEUED;
}

MessagePasser::PeerConnectionParameters* MessagePasser::FindPeerConnectionParameters(const network::IpAddressAndPort& address) {
	const auto predicate = [&address](const auto& pcp) { return pcp.peerAddress == address; };
	if (const auto it = std::ranges::find_if(_peerConnectionParameters, predicate); it != _peerConnectionParameters.end()) {
		return &*it;
	}
	return nullptr;
}
MessagePasser::PeerConnectionParameters& MessagePasser::FindOrCreatePeerConnectionParameters(const network::IpAddressAndPort& address) {
	auto* existing = FindPeerConnectionParameters(address);
	if (not existing) {
		_peerConnectionParameters.emplace_back(PeerConnectionParameters{.peerAddress = address});
		existing = &_peerConnectionParameters.back();
	}
	return *existing;
}

