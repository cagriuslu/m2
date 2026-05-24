#include <m2/multiplayer/TurnBasedServerUpdate.h>
#include <m2/Game.h>

m2::pb::TurnBasedNetworkMessage m2::GenerateServerUpdate(uint32_t& nextSequenceNo, const int turnHolderIndex, const bool shutdown) {
	// Prepare the TurnBasedServerUpdate except the receiver_index field
	pb::TurnBasedNetworkMessage message;
	message.set_game_hash(M2_GAME.Hash());
	message.set_sequence_no(nextSequenceNo++);
	message.mutable_server_update()->set_turn_holder_index(turnHolderIndex);
	for (const auto playerId : M2_LEVEL.multiPlayerObjectIds) {
		message.mutable_server_update()->add_player_object_ids(playerId);
	}
	M2_LEVEL.GetCharacterStorage().StoreAll(M2_LEVEL.objects, *message.mutable_server_update());
	message.mutable_server_update()->set_shutdown(shutdown);
	return message;
}
