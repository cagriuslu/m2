#include <m2/multi_player/lockstep/SmallMessagePasser.h>
#include <m2/Game.h>

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
			const auto msgSize = rit->second.ByteSizeLong();
			if (SmallMessageMaxSize() < I(msgSize)) {
				throw M2_ERROR("Small message is larger than allowed");
			}
			if (I(msgSize) + N_BYTES_ADDED_TO_HEADER_FOR_EACH_SMALL_MESSAGE <= spaceLeftInMessage) {
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
	packet.set_first_order_no(firstSmallMessageToSendReverseIt->first);
	// Limit iteration count
	m2Repeat(I(outgoingNackMessages.size())) {
		packet.add_small_messages()->CopyFrom(firstSmallMessageToSendReverseIt->second);
		if (firstSmallMessageToSendReverseIt == outgoingNackMessages.rbegin()) {
			break;
		}
		--firstSmallMessageToSendReverseIt; // Decrement reverse iterator to access the next message
	}
	return packet;
}
int32_t SmallMessagePasser::PeerConnectionParameters::GetMostRecentAck() const {
	if (gapHistory) {
		return gapHistory->oldestGapOrderNo + I(gapHistory->messagesSinceOldestGap.size()) - 1;
	}
	return lastOrderlyReceivedOrderNo;
}
int32_t SmallMessagePasser::PeerConnectionParameters::GetAckHistoryBits() const {
	if (gapHistory) {
		int32_t bits = 0, i = 0;
		for (auto rit = gapHistory->messagesSinceOldestGap.rbegin(); rit != gapHistory->messagesSinceOldestGap.rend(); ++rit) {
			if (*rit) {
				bits |= 1 << i; // Set the bit if a corresponding message is received
			}
			++i;
		}
		for (; i < 32; ++i) {
			bits |= 1 << i; // Set left over bits
		}
		return bits;
	}
	// If there are no gaps, everything up and including to lastOrderlyReceivedOrderNo is received
	return I(0xFFFFFFFF);
}
int32_t SmallMessagePasser::PeerConnectionParameters::GetOldestNack() const {
	if (gapHistory) {
		return gapHistory->oldestGapOrderNo;
	}
	return 0;
}

void SmallMessagePasser::ReadSmallMessages(std::queue<SmallMessageAndSender>& out) {
	// Assume the socket is readable
	auto recvResult = _socket.recv(_recvBuffer, sizeof(_recvBuffer));
	if (not recvResult) {
		LOG_WARN("Unable to recv", recvResult.error());
	}
	LOG_INFO("Received bytes", recvResult->first, recvResult->second);

	// TODO other tasks

	if (pb::LockstepUdpPacket packet; packet.ParseFromArray(_recvBuffer, recvResult->first)) {
		for (auto& smallMsg : packet.small_messages()) {
			out.emplace(std::move(smallMsg), std::make_pair(recvResult->second.first, recvResult->second.second));
		}
	} else {
		LOG_WARN("Unable to parse received message");
	}
}

m2::void_expected SmallMessagePasser::SendSmallMessage(SmallMessageAndReceiver&& in) {
	// Insert message to non-acknowledged messages
	auto& peerConnParams = FindOrCreatePeerConnectionParameters(in.receiver);
	peerConnParams.outgoingNackMessages.emplace_back(peerConnParams.nextOutgoingOrderNo++, std::move(in.smallMessage));
	// Prepare a packet specially for the peer
	const pb::LockstepUdpPacket packet = peerConnParams.CreateOutgoingPacketFromTailMessages();
	// Serialize and send
	const auto bytes = packet.SerializeAsString();
	const auto expectSuccess = _socket.send(in.receiver.first, in.receiver.second, bytes.data(), bytes.size());
	m2ReflectUnexpected(expectSuccess);
	LOG_DEBUG("Sent small message", in.receiver, packet.first_order_no(), packet.small_messages_size(), bytes.size());
	// Store current time for calculating retransmission later
	peerConnParams.lastMessageSentAt = Stopwatch{};
	return {};
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
		_peerConnectionParameters.emplace_back(PeerConnectionParameters{.peerAddress = address});
		existing = &_peerConnectionParameters.back();
	}
	return *existing;
}
