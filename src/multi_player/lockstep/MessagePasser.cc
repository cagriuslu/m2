#include <m2/multi_player/lockstep/MessagePasser.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

void_expected MessagePasser::ReadMessages(std::queue<MessageAndSender>& out) {
	const auto success = _smallMessagePasser.ReadSmallMessages(_receivedSmallMessages);
	m2ReflectUnexpected(success);

	while (not _receivedSmallMessages.empty()) {
		auto& [smallMsg, sender] = _receivedSmallMessages.front();
		m2SucceedOrThrowMessage(smallMsg.has_complete_message(), "Message doesn't have a complete message");
		// Check if the sequence number is sequential
		auto& peer = FindOrCreatePeerConnectionParameters(sender);
		const auto msgSequenceNo = I(smallMsg.message_sequence_no());
		m2SucceedOrThrowMessage(msgSequenceNo == peer.lastReceivedSequenceNo + 1, "Received out of order message");
		++peer.lastReceivedSequenceNo;
		// Return message
		out.push(MessageAndSender{
			.message = std::move(*smallMsg.mutable_complete_message()),
			.sender = sender
		});
		LOG_DEBUG("Returning message from peer, with sequence number", sender, msgSequenceNo);
		// Pop from queue
		_receivedSmallMessages.pop();
	}
	return {};
}

void_expected MessagePasser::QueueMessage(MessageAndReceiver&& in) {
	if (CompleteMessageMaxSize() < I(in.message.ByteSizeLong())) {
		return make_unexpected("Splitting large message into smaller chunks is not yet implemented");
	}
	// Look up peer and message sequence number
	auto& peerConnParams = FindOrCreatePeerConnectionParameters(in.receiver);
	const auto msgSequenceNo = peerConnParams.nextOutgoingSequenceNo++;
	// Queue message
	pb::LockstepSmallMessage smallMsg;
	smallMsg.set_message_sequence_no(msgSequenceNo);
	smallMsg.mutable_complete_message()->Swap(&in.message);
	_smallMessagePasser.QueueSmallMessage(SmallMessageAndReceiver{
		.smallMessage = std::move(smallMsg),
		.receiver = in.receiver
	});
	LOG_DEBUG("Queued outgoing message for peer, with sequence number", in.receiver, msgSequenceNo);
	return {};
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

