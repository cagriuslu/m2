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

	for (auto& char_variant : M2_LEVEL.characters) { // Iterate over characters
		auto* object_descriptor = message.mutable_server_update()->add_objects_with_character();

		// For any Character type
		std::visit(overloaded {
			[object_descriptor](const auto& v) {
				object_descriptor->set_object_id(v.Owner().GetId());
				object_descriptor->mutable_position()->CopyFrom(static_cast<pb::VecF>(v.Owner().InferPositionF()));
				object_descriptor->set_object_type(v.Owner().GetType());
				object_descriptor->set_parent_id(v.Owner().GetParentId());
				for (auto card_it = v.BeginCards(); card_it != v.EndCards(); ++card_it) {
					object_descriptor->add_cards(card_it->Type());
				}
				pb::for_each_enum_value<m2g::pb::VariableType>([&v, object_descriptor](m2g::pb::VariableType vt) {
					if (v.GetVariable(vt)) {
						auto* resource = object_descriptor->add_variables();
						resource->set_type(vt);
						resource->mutable_ife()->CopyFrom(static_cast<pb::IFE>(v.GetVariable(vt)));
					}
				});
			}
		}, char_variant);
	}
	message.mutable_server_update()->set_shutdown(shutdown);

	return message;
}
