#include <m2/multi_player/TurnBasedServerUpdate.h>
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
	M2_LEVEL.GetCharacterStorage().ForEachCharacter([&](const Character& chr) -> std::optional<std::monostate> {
		auto* objDesc = message.mutable_server_update()->add_objects_with_character();
		chr.Store(*objDesc);
		objDesc->set_object_id(chr.Owner().GetId());
		objDesc->mutable_position()->CopyFrom(static_cast<pb::VecF>(chr.Owner().InferPositionF()));
		objDesc->set_object_type(chr.Owner().GetType());
		objDesc->set_parent_id(chr.Owner().GetParentId());
		return std::nullopt;
	});
	message.mutable_server_update()->set_shutdown(shutdown);
	return message;
}
