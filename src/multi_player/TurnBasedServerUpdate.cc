#include <m2/multi_player/TurnBasedServerUpdate.h>
#include <m2/Game.h>

m2::pb::TurnBasedNetworkMessage m2::GenerateServerUpdate(uint32_t& nextSequenceNo, const int turnHolderIndex, const bool shutdown) {
	// Prepare the TurnBasedServerUpdate except the receiver_index field
	pb::TurnBasedNetworkMessage message;
	message.set_game_hash(M2_GAME.Hash());
	message.set_sequence_no(nextSequenceNo++);
	message.mutable_server_update()->set_turn_holder_index(turnHolderIndex);

	for (const auto playerId : M2G_PROXY.multiPlayerObjectIds) {
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
				for (auto item_it = v.BeginItems(); item_it != v.EndItems(); ++item_it) {
					const auto* item_ptr = item_it.Get();
					const auto* named_item_ptr = dynamic_cast<const Item*>(item_ptr);
					if (!named_item_ptr) {
						throw M2_ERROR("TurnBasedServerUpdate does not support unnamed items");
					}
					object_descriptor->add_named_items(named_item_ptr->Type());
				}
				pb::for_each_enum_value<m2g::pb::ResourceType>([&v, object_descriptor](m2g::pb::ResourceType rt) {
					if (v.HasResource(rt)) {
						auto* resource = object_descriptor->add_resources();
						resource->set_type(rt);
						resource->set_amount(v.GetResource(rt));
					}
				});
				pb::for_each_enum_value<m2g::pb::AttributeType>([&v, object_descriptor](m2g::pb::AttributeType at) {
					if (v.HasAttribute(at)) {
						auto* attribute = object_descriptor->add_attributes();
						attribute->set_type(at);
						attribute->set_amount(v.GetAttribute(at));
					}
				});
			}
		}, char_variant);
	}
	message.mutable_server_update()->set_shutdown(shutdown);

	return message;
}
