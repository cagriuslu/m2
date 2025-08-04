#include <m2/multi_player/lockstep/Message.h>
#include <Lockstep.pb.h>

int m2::multiplayer::lockstep::UdpPacketHeaderSize() {
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
int m2::multiplayer::lockstep::SmallMessageMaxSize() {
	static int size = []() {
		return MAX_UDP_PACKET_SIZE - UdpPacketHeaderSize() - N_BYTES_ADDED_TO_HEADER_FOR_EACH_SMALL_MESSAGE;
	}();
	return size;
}
int m2::multiplayer::lockstep::SmallMessageHeaderSize() {
	static int size = []() {
		pb::LockstepSmallMessage msg;
		msg.set_message_sequence_no(INT32_MAX);
		msg.set_reconstructed_message_size(INT32_MAX);
		return msg.ByteSizeLong();
	}();
	return size;
}
int m2::multiplayer::lockstep::CompleteMessageMaxSize() {
	static int size = []() {
		constexpr auto bytesAddedForCompleteMessage = 4;
		return SmallMessageMaxSize() - SmallMessageHeaderSize() - bytesAddedForCompleteMessage;
	}();
	return size;
}
