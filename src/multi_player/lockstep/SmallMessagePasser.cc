#include <m2/multi_player/lockstep/SmallMessagePasser.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <ranges>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

namespace {
	constexpr int N_ATTEMPTS_TO_READ_PACKETS = 8;
	constexpr std::chrono::milliseconds RETRANSMISSION_TIMEOUT{250};
	constexpr std::chrono::milliseconds KEEP_ALIVE_PING_TIMEOUT{5000};
}

const ConnectionStatistics& SmallMessagePasser::PeerConnectionParameters::GetConnectionStatistics() const {
	return connectionStatistics;
}
network::OrderNo SmallMessagePasser::PeerConnectionParameters::GetMostRecentAck() const {
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
			throw M2_ERROR("Small message with order number less than 1 has ended up in the queue");
		}
		return I(messagesSinceGap.begin()->second.order_no()) - 1;
	}
	return 0;
}
const Stopwatch* SmallMessagePasser::PeerConnectionParameters::GetTimeSinceOldestNackTransmission() const {
	if (not outgoingNackMessages.empty()) {
		return &outgoingNackMessages.front().lastSentAt;
	}
	return nullptr;
}

pb::LockstepUdpPacket SmallMessagePasser::PeerConnectionParameters::CreateOutgoingPacketFromTailMessagesAndUpdateTimers() {
	pb::LockstepUdpPacket packet;
	packet.set_game_hash(M2_GAME.Hash());
	packet.set_most_recent_ack(GetMostRecentAck());
	packet.set_ack_history_bits(GetAckHistoryBits());
	packet.set_oldest_nack(GetOldestNack());

	// Fill packet with messages in reverse order
	auto spaceLeftInMessage = MAX_UDP_PACKET_SIZE - UdpPacketHeaderSize();
	for (auto& [smallMessage, lastSentAt] : std::ranges::reverse_view{outgoingNackMessages}) {
		const auto occupiedSize = I(smallMessage.ByteSizeLong()) + N_BYTES_ADDED_TO_HEADER_FOR_EACH_SMALL_MESSAGE;
		if (spaceLeftInMessage < occupiedSize) {
			break; // Message won't fit into this packet
		}
		spaceLeftInMessage -= occupiedSize;
		packet.add_small_messages()->CopyFrom(smallMessage);
		// Update timer
		lastSentAt = Stopwatch{};
	}

	// Reverse the message order
	std::ranges::reverse(*packet.mutable_small_messages());

	dirtyOutgoing = false;
	dirtyAck = false;

	return packet;
}
pb::LockstepUdpPacket SmallMessagePasser::PeerConnectionParameters::CreateOutgoingPacketFromHeadMessagesAndUpdateTimers() {
	pb::LockstepUdpPacket packet;
	packet.set_game_hash(M2_GAME.Hash());
	packet.set_most_recent_ack(GetMostRecentAck());
	packet.set_ack_history_bits(GetAckHistoryBits());
	packet.set_oldest_nack(GetOldestNack());

	// Fill packet with messages
	auto spaceLeftInMessage = MAX_UDP_PACKET_SIZE - UdpPacketHeaderSize();
	for (auto& [smallMessage, lastSentAt] : outgoingNackMessages) {
		const auto occupiedSize = I(smallMessage.ByteSizeLong()) + N_BYTES_ADDED_TO_HEADER_FOR_EACH_SMALL_MESSAGE;
		if (spaceLeftInMessage < occupiedSize) {
			break; // Message won't fit into this packet
		}
		spaceLeftInMessage -= occupiedSize;
		packet.add_small_messages()->CopyFrom(smallMessage);
		// Update timer
		lastSentAt = Stopwatch{};
	}

	dirtyAck = false;

	return packet;
}
pb::LockstepUdpPacket SmallMessagePasser::PeerConnectionParameters::CreateOutgoingKeepAlivePacket() {
	pb::LockstepUdpPacket packet;
	packet.set_game_hash(M2_GAME.Hash());
	packet.set_most_recent_ack(GetMostRecentAck());
	packet.set_ack_history_bits(GetAckHistoryBits());
	packet.set_oldest_nack(GetOldestNack());

	// Keep-alive packet doesn't contain any messages

	dirtyAck = false;

	return packet;
}
void SmallMessagePasser::PeerConnectionParameters::QueueOutgoingMessage(pb::LockstepSmallMessage&& in) {
	// Insert message to non-acknowledged messages
	const auto orderNo = nextOutgoingOrderNo++;
	LOG_NETWORK("Queueing outgoing small message for peer, with order number", peerAddress, orderNo);
	in.set_order_no(orderNo); // Assign order number
	outgoingNackMessages.emplace_back(std::move(in), Stopwatch{});
	connectionStatistics.IncrementOutgoingSmallMessageCount();
	dirtyOutgoing = true;
}
void SmallMessagePasser::PeerConnectionParameters::ProcessPeerAcks(const int32_t mostRecentAck, int32_t ackHistoryBits, const int32_t oldestNack) {
	const auto nackCountBefore = outgoingNackMessages.size();

	std::vector<network::OrderNo> ackedOrderNos;

	// Discard messages up to the oldest NACK
	if (oldestNack) {
		std::erase_if(outgoingNackMessages, [&](const auto& msg) {
			const auto shouldDiscard = I(msg.smallMessage.order_no()) < oldestNack;
			if (shouldDiscard) {
				ackedOrderNos.emplace_back(msg.smallMessage.order_no());
			}
			return shouldDiscard;
		});
	}
	// Discard mostRecentAck
	if (mostRecentAck) {
		std::erase_if(outgoingNackMessages, [&](const auto& msg) {
			const auto shouldDiscard = I(msg.smallMessage.order_no()) == mostRecentAck;
			if (shouldDiscard) {
				ackedOrderNos.emplace_back(msg.smallMessage.order_no());
			}
			return shouldDiscard;
		});
	}
	// Iterate over bits of ackHistoryBits from the least significant
	for (int i = 0; i < 32; ++i) {
		if (const auto orderNoOfBit = mostRecentAck - i - 1; 0 < orderNoOfBit) {
			if (ackHistoryBits & 0x1) {
				std::erase_if(outgoingNackMessages, [&](const auto& msg) {
					const auto shouldDiscard = I(msg.smallMessage.order_no()) == orderNoOfBit;
					if (shouldDiscard) {
						ackedOrderNos.emplace_back(msg.smallMessage.order_no());
					}
					return shouldDiscard;
				});
			}
		}
		ackHistoryBits >>= 1;
	}

	const auto nackCountAfter = outgoingNackMessages.size();
	const auto ackCount = nackCountBefore - nackCountAfter;
	connectionStatistics.IncrementAckedOutgoingSmallMessageCount(I(ackCount));

	if (ackCount) {
		LOG_NETWORK("Peer acknowledged small messages with order number", peerAddress, ackedOrderNos);
	}
}
void SmallMessagePasser::PeerConnectionParameters::ProcessReceivedMessages(google::protobuf::RepeatedPtrField<pb::LockstepSmallMessage>* smallMessages,
		std::queue<SmallMessageAndSender>& out) {
	lastAnyMessageReceivedAt = Stopwatch{};

	if (smallMessages->empty()) {
		LOG_NETWORK("Received empty packet from peer", peerAddress);
		return;
	}

	const auto outOfOrderMessageCountBefore = messagesSinceGap.size();

	// Move all messages to messagesSinceGap
	for (auto& msg : *smallMessages) {
		if (const auto msgOrderNo = msg.order_no(); lastOrderlyReceivedOrderNo < msgOrderNo) {
			if (const auto [_, inserted] = messagesSinceGap.emplace(msgOrderNo, std::move(msg)); inserted) {
				LOG_NETWORK("Received new small message from peer, with order number", peerAddress, msgOrderNo);
			}
		}
	}
	smallMessages->Clear();

	const auto outOfOrderMessageCountAfter = messagesSinceGap.size();
	const auto lastOrderlyReceivedOrderNoBefore = lastOrderlyReceivedOrderNo;

	// Iterate messages to check for orderly messages
	for (auto it = messagesSinceGap.begin(); it != messagesSinceGap.end(); ) {
		if (const auto msgOrderNo = it->first; msgOrderNo == lastOrderlyReceivedOrderNo + 1) {
			out.emplace(std::move(it->second), peerAddress);
			++lastOrderlyReceivedOrderNo;
			it = messagesSinceGap.erase(it);
			LOG_NETWORK("Returning small message from peer, with order number", peerAddress, msgOrderNo);
		} else {
			break;
		}
	}

	const auto lastOrderlyReceivedOrderNoAfter = lastOrderlyReceivedOrderNo;

	dirtyAck = outOfOrderMessageCountBefore != outOfOrderMessageCountAfter
		|| lastOrderlyReceivedOrderNoBefore != lastOrderlyReceivedOrderNoAfter;
}

const ConnectionStatistics* SmallMessagePasser::GetConnectionStatistics(const network::IpAddressAndPort& address) const {
	if (const auto* existing = FindPeerConnectionParameters(address)) {
		return &existing->GetConnectionStatistics();
	}
	return nullptr;
}

void_expected SmallMessagePasser::ReadSmallMessages(std::queue<SmallMessageAndSender>& out) {
	m2Repeat(N_ATTEMPTS_TO_READ_PACKETS) {
		const auto isReadable = network::Select::IsSocketReadable(&_socket);
		m2ReflectUnexpected(isReadable);
		if (not isReadable.value()) {
			return {};
		}

		auto recvResult = _socket.Recv(_recvBuffer, sizeof(_recvBuffer));
		m2ReflectUnexpected(recvResult);

		PeerConnectionParameters* peer;
		if (peer = FindPeerConnectionParameters(recvResult->second); not peer) {
			if (_blockUnknownConnections) {
				LOG_NETWORK("Dropping packet from unknown source", recvResult->second);
				continue;
			}
			LOG_NETWORK("Accepting packet from unknown source, of size", recvResult->second, recvResult->first);
			peer = &FindOrCreatePeerConnectionParameters(recvResult->second);
		} else {
			LOG_NETWORK("Received packet from peer, of size", recvResult->second, recvResult->first);
		}

		if (pb::LockstepUdpPacket packet; packet.ParseFromArray(_recvBuffer, recvResult->first)) {
			if (I(packet.game_hash()) != M2_GAME.Hash()) {
				LOG_DEBUG("Game hash mismatch");
				continue;
			}
			// TODO check version
			peer->ProcessPeerAcks(I(packet.most_recent_ack()), I(packet.ack_history_bits()), I(packet.oldest_nack()));
			peer->ProcessReceivedMessages(packet.mutable_small_messages(), out);
		} else {
			LOG_WARN("Unable to parse received message");
		}
	}
	return {};
}
void SmallMessagePasser::QueueSmallMessage(SmallMessageAndReceiver&& in) {
	if (SmallMessageMaxSize() < I(in.smallMessage.ByteSizeLong())) {
		throw M2_ERROR("Small message is larger than allowed");
	}
	// Insert message to non-acknowledged messages
	auto& peerConnParams = FindOrCreatePeerConnectionParameters(in.receiver);
	peerConnParams.QueueOutgoingMessage(std::move(in.smallMessage));
}
void_expected SmallMessagePasser::SendOutgoingPackets() {
	for (auto& peer : _peerConnectionParameters) {
		// (Re)send latest messages if there is new messages or ACK to be sent
		if (peer.IsDirty()) {
			const auto isWritable = network::Select::IsSocketWritable(&_socket);
			m2ReflectUnexpected(isWritable);
			if (isWritable.value()) {
				const auto packet = peer.CreateOutgoingPacketFromTailMessagesAndUpdateTimers();
				const auto bytes = packet.SerializeAsString();
				const auto success = _socket.Send(peer.GetPeerAddress(), bytes.data(), bytes.size());
				m2ReflectUnexpected(success);
				LOG_NETWORK("Sent fresh packet to peer, of size, with small message order numbers", peer.GetPeerAddress(), bytes.size(), packet.small_messages());
				peer.MarkAnyMessageSent();
			}
		}

		// Resend oldest messages if enough time has passed without ACK
		if (const auto* timeSinceTx = peer.GetTimeSinceOldestNackTransmission(); timeSinceTx && timeSinceTx->HasTimePassed(RETRANSMISSION_TIMEOUT)) {
			const auto isWritable = network::Select::IsSocketWritable(&_socket);
			m2ReflectUnexpected(isWritable);
			if (isWritable.value()) {
				const auto packet = peer.CreateOutgoingPacketFromHeadMessagesAndUpdateTimers();
				const auto bytes = packet.SerializeAsString();
				const auto success = _socket.Send(peer.GetPeerAddress(), bytes.data(), bytes.size());
				m2ReflectUnexpected(success);
				LOG_NETWORK("Sent retransmission packet to peer, of size, with small message count", peer.GetPeerAddress(), bytes.size(), packet.small_messages());
				peer.MarkAnyMessageSent();
			}
		}

		// Ping the peer if no other messages were sent to the peer for a long time
		if (peer.GetTimeSinceAnySentMessage().HasTimePassed(KEEP_ALIVE_PING_TIMEOUT)) {
			const auto isWritable = network::Select::IsSocketWritable(&_socket);
			m2ReflectUnexpected(isWritable);
			if (isWritable.value()) {
				const auto packet = peer.CreateOutgoingKeepAlivePacket();
				const auto bytes = packet.SerializeAsString();
				const auto success = _socket.Send(peer.GetPeerAddress(), bytes.data(), bytes.size());
				m2ReflectUnexpected(success);
				LOG_NETWORK("Sent keep-alive packet to peer", peer.GetPeerAddress());
				peer.MarkAnyMessageSent();
			}
		}
	}
	return {};
}
void SmallMessagePasser::Flush() {
	// TODO
}

const SmallMessagePasser::PeerConnectionParameters* SmallMessagePasser::FindPeerConnectionParameters(const network::IpAddressAndPort& address) const {
	const auto predicate = [&address](const auto& pcp) { return pcp.GetPeerAddress() == address; };
	if (const auto it = std::ranges::find_if(_peerConnectionParameters, predicate); it != _peerConnectionParameters.end()) {
		return &*it;
	}
	return nullptr;
}
SmallMessagePasser::PeerConnectionParameters* SmallMessagePasser::FindPeerConnectionParameters(const network::IpAddressAndPort& address) {
	const auto predicate = [&address](const auto& pcp) { return pcp.GetPeerAddress() == address; };
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

std::string m2::ToString(const google::protobuf::RepeatedPtrField<pb::LockstepSmallMessage>& smallMessages) {
	std::string s = "[";
	for (const auto& msg : smallMessages) {
		s += ToString(msg.order_no()) + ",";
	}
	s += "]";
	return s;
}
