#include <m2/multi_player/lockstep/SmallMessagePasser.h>
#include <m2/Game.h>
#include <ranges>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

pb::LockstepUdpPacket SmallMessagePasser::PeerConnectionParameters::CreateOutgoingPacketFromTailMessages() const {
	// Find the first message after which all messages can fit into the UDP packet
	auto firstSmallMessageToSendReverseIt = [this]() {
		auto spaceLeftInMessage = MAX_UDP_PACKET_SIZE - UdpPacketHeaderSize();
		auto rit = outgoingNackMessages.rbegin();
		for (; rit != outgoingNackMessages.rend(); ++rit) {
			// Assert message size as well
			const auto msgSize = I(rit->smallMessage.ByteSizeLong());
			if (SmallMessageMaxSize() < msgSize) {
				throw M2_ERROR("Small message is larger than allowed");
			}
			if (msgSize + N_BYTES_ADDED_TO_HEADER_FOR_EACH_SMALL_MESSAGE <= spaceLeftInMessage) {
				spaceLeftInMessage -= msgSize + N_BYTES_ADDED_TO_HEADER_FOR_EACH_SMALL_MESSAGE;
				continue;
			}
			// Current message won't fit
			return rit - 1;
		}
		// All message fit
		return rit - 1;
	}();

	pb::LockstepUdpPacket packet;
	packet.set_game_hash(M2_GAME.Hash());
	packet.set_most_recent_ack(GetMostRecentAck());
	packet.set_ack_history_bits(GetAckHistoryBits());
	packet.set_oldest_nack(GetOldestNack());
	// Limit iteration count
	m2Repeat(I(outgoingNackMessages.size())) {
		packet.add_small_messages()->CopyFrom(firstSmallMessageToSendReverseIt->smallMessage);
		if (firstSmallMessageToSendReverseIt == outgoingNackMessages.rbegin()) {
			break;
		}
		--firstSmallMessageToSendReverseIt; // Decrement reverse iterator to access the next message
	}
	return packet;
}
int32_t SmallMessagePasser::PeerConnectionParameters::GetMostRecentAck() const {
	if (not messagesSinceGap.empty()) {
		return messagesSinceGap.rbegin()->second.order_no();
	}
	return lastOrderlyReceivedOrderNo;
}
int32_t SmallMessagePasser::PeerConnectionParameters::GetAckHistoryBits() const {
	if (not messagesSinceGap.empty()) {
		int32_t bits = 0;
		auto orderNo = GetMostRecentAck() - 1;
		for (int i = 0; i < 32; ++i, --orderNo) {
			if (lastOrderlyReceivedOrderNo < orderNo) {
				if (messagesSinceGap.contains(orderNo)) {
					bits |= 1 << i; // Set the bit if the corresponding message is received
				}
			} else {
				bits |= 1 << i; // Messages before and including lastOrderlyReceivedOrderNo are received
			}
		}
		return bits;
	}
	// If there are no gaps, everything up and including to lastOrderlyReceivedOrderNo is received
	return I(0xFFFFFFFF);
}
int32_t SmallMessagePasser::PeerConnectionParameters::GetOldestNack() const {
	if (not messagesSinceGap.empty()) {
		if (messagesSinceGap.begin()->second.order_no() < 1) {
			throw M2_ERROR("Small message with order no less than 1 ended up in the queue");
		}
		return messagesSinceGap.begin()->second.order_no() - 1;
	}
	return 0;
}

void SmallMessagePasser::PeerConnectionParameters::ProcessPeerAcks(const int32_t mostRecentAck, int32_t ackHistoryBits, const int32_t oldestNack) {
	// Discard messages up to the oldest NACK
	if (oldestNack) {
		std::erase_if(outgoingNackMessages, [&](const auto& msg) {
			return I(msg.smallMessage.order_no()) < oldestNack;
		});
	}
	// Discard mostRecentAck
	if (mostRecentAck) {
		std::erase_if(outgoingNackMessages, [&](const auto& msg) {
			return I(msg.smallMessage.order_no()) == mostRecentAck;
		});
	}
	// Iterate over bits of ackHistoryBits from the least significant
	for (int i = 0; i < 32; ++i) {
		if (const auto orderNoOfBit = mostRecentAck - i - 1; 0 < orderNoOfBit) {
			if (ackHistoryBits & 0x1) {
				std::erase_if(outgoingNackMessages, [&](const auto& msg) {
					return I(msg.smallMessage.order_no()) == orderNoOfBit;
				});
			}
		}
		ackHistoryBits >>= 1;
	}
}
void SmallMessagePasser::PeerConnectionParameters::ProcessReceivedMessages(google::protobuf::RepeatedPtrField<pb::LockstepSmallMessage>* smallMessages,
		std::queue<SmallMessageAndSender>& out) {
	// Move all messages to messagesSinceGap
	for (auto& msg : *smallMessages) {
		const auto orderNo = msg.order_no();
		messagesSinceGap.emplace(orderNo, std::move(msg));
	}
	smallMessages->Clear();
	// Iterate messages to check for orderly messages
	for (auto it = messagesSinceGap.begin(); it != messagesSinceGap.end(); ) {
		if (I(it->second.order_no()) == lastOrderlyReceivedOrderNo + 1) {
			out.emplace(std::move(it->second));
			++lastOrderlyReceivedOrderNo;
			it = messagesSinceGap.erase(it);
		} else {
			break;
		}
	}
	sendAck = true;
}

ConnectionStatistics* SmallMessagePasser::GetConnectionStatistics(const network::IpAddressAndPort& address) {
	if (auto* existing = FindPeerConnectionParameters(address)) {
		return &existing->connectionStatistics;
	}
	return nullptr;
}

void SmallMessagePasser::ReadSmallMessages(std::queue<SmallMessageAndSender>& out) {
	// Assuming the socket is readable
	auto recvResult = _socket.Recv(_recvBuffer, sizeof(_recvBuffer));
	if (not recvResult) {
		LOG_WARN("Unable to recv", recvResult.error());
		return;
	}
	LOG_INFO("Received bytes", recvResult->first, recvResult->second);

	PeerConnectionParameters* peer;
	if (peer = FindPeerConnectionParameters(recvResult->second); not peer) {
		if (_blockUnknownConnections) {
			LOG_DEBUG("Dropping message from unknown source", recvResult->second);
			return;
		}
		LOG_INFO("Accepting message from unknown source", recvResult->second);
		peer = &FindOrCreatePeerConnectionParameters(recvResult->second);
	}

	if (pb::LockstepUdpPacket packet; packet.ParseFromArray(_recvBuffer, recvResult->first)) {
		if (I(packet.game_hash()) != M2_GAME.Hash()) {
			LOG_DEBUG("Game hash mismatch");
			return;
		}
		// TODO check version
		peer->ProcessPeerAcks(I(packet.most_recent_ack()), I(packet.ack_history_bits()), I(packet.oldest_nack()));
		peer->ProcessReceivedMessages(packet.mutable_small_messages(), out);
	} else {
		LOG_WARN("Unable to parse received message");
	}
}
void_expected SmallMessagePasser::SendSmallMessage(SmallMessageAndReceiver&& in) {
	// Insert message to non-acknowledged messages
	auto& peerConnParams = FindOrCreatePeerConnectionParameters(in.receiver);
	in.smallMessage.set_order_no(peerConnParams.nextOutgoingOrderNo++); // Assign the order no
	peerConnParams.outgoingNackMessages.emplace_back(std::move(in.smallMessage), Stopwatch{});
	// Prepare a packet specially for the peer
	const pb::LockstepUdpPacket packet = peerConnParams.CreateOutgoingPacketFromTailMessages();
	// Serialize and send
	const auto bytes = packet.SerializeAsString();
	const auto expectSuccess = _socket.Send(in.receiver, bytes.data(), bytes.size());
	m2ReflectUnexpected(expectSuccess);
	LOG_DEBUG("Sent small message", in.receiver, bytes.size(), packet.small_messages_size());
	// Store current time for calculating retransmission later
	peerConnParams.lastMessageSentAt = Stopwatch{};
	return {};
}
void SmallMessagePasser::SendRetransmissionsAndAcks() {
	// TODO
}

SmallMessagePasser::PeerConnectionParameters* SmallMessagePasser::FindPeerConnectionParameters(const network::IpAddressAndPort& address) {
	const auto predicate = [&address](const auto& pcp) { return pcp.peerAddress == address; };
	if (const auto it = std::ranges::find_if(_peerConnectionParameters, predicate); it != _peerConnectionParameters.end()) {
		return &*it;
	}
	return nullptr;
}
SmallMessagePasser::PeerConnectionParameters& SmallMessagePasser::FindOrCreatePeerConnectionParameters(const network::IpAddressAndPort& address) {
	auto* existing = FindPeerConnectionParameters(address);
	if (not existing) {
		_peerConnectionParameters.emplace_back(address);
		existing = &_peerConnectionParameters.back();
	}
	return *existing;
}
