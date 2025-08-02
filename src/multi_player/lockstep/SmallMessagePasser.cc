#include <m2/multi_player/lockstep/SmallMessagePasser.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

void SmallMessagePasser::ReadSmallMessages(std::queue<SmallMessageAndSender>& out) {
	// Assume the socket is readable
	auto recvResult = _socket.recv(_recvBuffer, sizeof(_recvBuffer));
	if (not recvResult) {
		LOG_WARN("Unable to recv", recvResult.error());
	}
	LOG_INFO("Received bytes", recvResult->first, recvResult->second);

	if (pb::LockstepUdpPacket packet; packet.ParseFromArray(_recvBuffer, recvResult->first)) {
		for (auto& smallMsg : packet.small_messages()) {
			out.emplace(std::move(smallMsg), std::make_pair(recvResult->second.first, recvResult->second.second), 0); // TODO order no
		}
	} else {
		LOG_WARN("Unable to parse received message");
	}
}

void SmallMessagePasser::SendSmallMessage(SmallMessageAndReceiver&& in) {
	pb::LockstepUdpPacket packet;
	packet.set_game_hash(M2_GAME.Hash());
	const auto bytes = packet.SerializeAsString();
	const auto expectNBytes = _socket.send(in.receiver.first, in.receiver.second, bytes.data(), bytes.size());
	if (not expectNBytes) {
		LOG_WARN("Unable to send packet", expectNBytes.error());
	}
	LOG_INFO("Sent bytes", *expectNBytes);
}
