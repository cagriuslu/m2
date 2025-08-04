#include <m2/multi_player/lockstep/Message.h>
#include <Lockstep.pb.h>

int m2::multiplayer::lockstep::LockstepUdpPacketHeaderSize() {
	static int size = []() {
		pb::LockstepUdpPacket packet;
		packet.set_game_hash(INT32_MAX);
		packet.set_most_recent_ack(INT32_MAX);
		packet.set_ack_history_bits(INT32_MAX);
		packet.set_oldest_nack(INT32_MAX);
		packet.set_first_order_no(INT32_MAX);
		return packet.ByteSizeLong();
	}();
	return size;
}

int m2::multiplayer::lockstep::LockstepSmallMessageMaxSize() {
	static int size = []() {
		return MAX_UDP_PACKET_SIZE - LockstepUdpPacketHeaderSize() - EACH_SMALL_MESSAGE_HEADER_SIZE;
	}();
	return size;
}
