#include <../../include/m2/multi_player/TurnBasedRealClientThread.h>
#include <m2/Game.h>
#include <m2/Log.h>

namespace {
	template <typename NamedItemListT, typename ResourceListT, typename AttributeListT>
	void update_character(m2::Character* c, const NamedItemListT& named_items, const ResourceListT& resources, const AttributeListT& attributes) {
		// Update items
		c->ClearItems();
		for (auto named_item_type : named_items) {
			c->AddNamedItemWithoutBenefits(M2_GAME.GetNamedItem(static_cast<m2g::pb::ItemType>(named_item_type)));
		}
		// Update resources
		c->ClearResources();
		for (const auto& resource : resources) {
			c->AddResource(resource.type(), m2::GetResourceAmount(resource));
		}
		// Update attributes
		c->ClearAttributes();
		for (const auto& attribute : attributes) {
			c->SetAttribute(attribute.type(), attribute.amount());
		}
	}
}

m2::network::TurnBasedRealClientThread::TurnBasedRealClientThread(std::string addr)
	: detail::TurnBasedClientThreadBase{std::move(addr), true} {
	latch();
}

const char* m2::network::TurnBasedRealClientThread::thread_name() const {
	return "RC";
}

int m2::network::TurnBasedRealClientThread::total_player_count() {
	if (_last_processed_server_update) {
		// Game is already running
		return _last_processed_server_update->second.player_object_ids_size();
	} else if (auto unprocessed_server_update = locked_peek_server_update()) {
		// Game is not yet running, but a TurnBasedServerUpdate is received (but not yet processed).
		return unprocessed_server_update->player_object_ids_size();
	} else {
		throw M2_ERROR("Game not yet started");
	}
}
std::optional<m2g::pb::TurnBasedServerCommand> m2::network::TurnBasedRealClientThread::pop_server_command() {
	if (auto serverCommand = locked_pop_server_command()) {
		return serverCommand->second;
	}
	return std::nullopt;
}
int m2::network::TurnBasedRealClientThread::self_index() {
	if (_last_processed_server_update) {
		// Game is already running
		return _last_processed_server_update->second.receiver_index();
	} else if (auto unprocessed_server_update = locked_peek_server_update()) {
		// Game is not yet running, but a TurnBasedServerUpdate is received (but not yet processed).
		return unprocessed_server_update->receiver_index();
	} else {
		throw M2_ERROR("Game not yet started");
	}
}

int m2::network::TurnBasedRealClientThread::turn_holder_index() {
	if (_last_processed_server_update) {
		// Game is already running
		return _last_processed_server_update->second.turn_holder_index();
	} else if (auto unprocessed_server_update = locked_peek_server_update()) {
		// Game is not yet running, but a TurnBasedServerUpdate is received (but not yet processed).
		return unprocessed_server_update->turn_holder_index();
	} else {
		throw M2_ERROR("Game not yet started");
	}
}

m2::expected<std::pair<m2::network::ServerUpdateStatus,m2::network::SequenceNo>> m2::network::TurnBasedRealClientThread::process_server_update() {
	TRACE_FN();

	auto unprocessed_server_update = locked_pop_server_update();
	if (not unprocessed_server_update) {
		LOG_TRACE("No TurnBasedServerUpdate to process");
		return std::make_pair(ServerUpdateStatus::NOT_FOUND, -1);
	}

	LOG_DEBUG("Shifting server update: prev << last << unprocessed << null");
	if (_last_processed_server_update) {
		_prev_processed_server_update = std::move(_last_processed_server_update);
	}
	_last_processed_server_update = std::move(unprocessed_server_update);

	if (not _prev_processed_server_update) {
		LOG_DEBUG("Processing first TurnBasedServerUpdate");
		// This will be the first TurnBasedServerUpdate, that started the game.
		// Only do verification as level initialization should have initialized the same exact game state
		const auto& server_update = _last_processed_server_update->second;

		if (M2G_PROXY.multiPlayerObjectIds.size() != Z(server_update.player_object_ids_size())) {
			return make_unexpected("Server and local player count doesn't match");
		}

		if (M2_LEVEL.playerId != M2G_PROXY.multiPlayerObjectIds[server_update.receiver_index()]) {
			return make_unexpected("Player ID doesn't match the ID found in local player list");
		}

		if (M2_LEVEL.characters.Size() != Z(server_update.objects_with_character_size())) {
			return make_unexpected("Server and local have different number of characters");
		}

		int i = 0;
		for (auto char_it = M2_LEVEL.characters.begin(); char_it != M2_LEVEL.characters.end() && i < server_update.objects_with_character_size(); ++char_it, ++i) {
			auto server_character = server_update.objects_with_character(i);
			auto success = std::visit(overloaded {
				[this, &server_character](const auto& v) -> m2::void_expected {
					if (v.Owner().position != VecF{server_character.position()}) {
						return make_unexpected("Server and local position mismatch");
					}
					if (v.Owner().GetType() != server_character.object_type()) {
						return make_unexpected("Server and local object type mismatch");
					}
					if (std::distance(v.BeginItems(), v.EndItems()) != server_character.named_items_size()) {
						return make_unexpected("Server and local item count mismatch");
					}
					// TODO other checks

					// Map server ObjectIDs to local ObjectIDs
					_server_to_local_map[server_character.object_id()] = std::make_pair(v.OwnerId(), true);

					return {};
				}
			}, *char_it);
			m2ReflectUnexpected(success);
		}

		// Check if server_to_local_map contains all the players in the game
		for (auto player_object_id : server_update.player_object_ids()) {
			if (auto it = _server_to_local_map.find(player_object_id); it == _server_to_local_map.end()) {
				return make_unexpected("Server to local object ID map does not contain a player's ID");
			}
		}

		if (server_update.shutdown()) {
			return make_unexpected("Unexpected shutdown flag in first TurnBasedServerUpdate");
		}

		return std::make_pair(ServerUpdateStatus::PROCESSED, _last_processed_server_update->first); // Successfully processed the first TurnBasedServerUpdate
	}

	LOG_DEBUG("Processing TurnBasedServerUpdate");
	const auto& server_update = _last_processed_server_update->second;
	{
		const auto& prev_server_update = _prev_processed_server_update->second;
		// Check that the player IDs haven't changed
		if (prev_server_update.player_object_ids_size() != server_update.player_object_ids_size()) {
			return make_unexpected("Number of players have changed");
		}
		for (int i = 0; i < prev_server_update.player_object_ids_size(); ++i) {
			if (prev_server_update.player_object_ids(i) != server_update.player_object_ids(i)) {
				return make_unexpected("A player's ID has changed");
			}
		}
	}

	// Mark local objects as unvisited
	std::ranges::for_each(_server_to_local_map, [](auto& kv_pair) { kv_pair.second.second = false; });

	struct ObjectToCreate {
		ObjectId server_object_id;
		pb::VecF position;
		m2g::pb::ObjectType object_type;
		ObjectId server_object_parent_id;
		std::vector<m2g::pb::ItemType> named_items;
		std::vector<m2::pb::Resource> resources;
		std::vector<m2::pb::Attribute> attributes;
	};
	std::vector<ObjectToCreate> objects_to_be_created;

	// Iterate over TurnBasedServerUpdate objects w/ character
	for (const auto& object_desc : server_update.objects_with_character()) {
		if (auto it = _server_to_local_map.find(object_desc.object_id()); it != _server_to_local_map.end()) {
			LOG_TRACE("Server object is still alive", object_desc.object_id(), it->first);
			// Update the character
			auto* character = M2_LEVEL.objects.Get(it->second.first)->TryGetCharacter();
			update_character(character, object_desc.named_items(), object_desc.resources(), object_desc.attributes());
			// Mark object as visited
			it->second.second = true;
		} else {
			auto int_list_to_item_list = [](const auto& begin, const auto& end) {
				std::vector<m2g::pb::ItemType> item_list;
				for (auto it = begin; it != end; ++it) { item_list.emplace_back(static_cast<m2g::pb::ItemType>(*it)); }
				return item_list;
			};

			// Add details about the object that'll be created into a list
			objects_to_be_created.push_back({object_desc.object_id(), object_desc.position(), object_desc.object_type(),
											 object_desc.parent_id(), int_list_to_item_list(object_desc.named_items().begin(), object_desc.named_items().end()),
											 {object_desc.resources().begin(), object_desc.resources().end()}, {object_desc.attributes().begin(), object_desc.attributes().end()}});
		}
	}

	// Create new objects. This is more complicated than updating existing objects because of the parent relationships.
	// - Objects without a parent can be created directly.
	// - Objects with a parent which is already created can also be created directly, referring to the local object as the parent.
	// - Objects with a parent which is not yet created cannot be created directly. We need to iterate the objects_to_be_created
	//   list over and over again until all parents are created.
	while (not objects_to_be_created.empty()) {
		// Loop until all objects are created

		// Iterate over objects to be created
		auto it = objects_to_be_created.begin();
		while (it != objects_to_be_created.end()) {
			// If the new object has no parent
			if (it->server_object_parent_id == 0) {
				// Simply, create the object
				LOG_DEBUG("Server has created an object", it->server_object_id);
				auto obj_it = m2::CreateObject(m2::VecF{it->position}, it->object_type, 0);
				auto load_result = M2G_PROXY.init_server_update_fg_object(*obj_it, it->named_items, it->resources);
				m2ReflectUnexpected(load_result);
				// Update the character
				auto* character = obj_it->TryGetCharacter();
				update_character(character, it->named_items, it->resources, it->attributes);
				// Add object to the map, marked as visited
				_server_to_local_map[it->server_object_id] = std::make_pair(obj_it.GetId(), true);
				// Delete the object details from the objects_to_be_created vector
				it = objects_to_be_created.erase(it);
			} else if (auto parent_it = _server_to_local_map.find(it->server_object_parent_id); parent_it != _server_to_local_map.end()) {
				// If the object has a parent that's already created, create the object by looking up the corresponding parent
				LOG_DEBUG("Server has created an object", it->server_object_id);
				auto obj_it = m2::CreateObject(m2::VecF{it->position}, it->object_type, parent_it->second.first);
				auto load_result = M2G_PROXY.init_server_update_fg_object(*obj_it, it->named_items, it->resources);
				m2ReflectUnexpected(load_result);
				// Update the character
				auto* character = obj_it->TryGetCharacter();
				update_character(character, it->named_items, it->resources, it->attributes);
				// Add object to the map, marked as visited
				_server_to_local_map[it->server_object_id] = std::make_pair(obj_it.GetId(), true);
				// Delete the object details from the objects_to_be_created vector
				it = objects_to_be_created.erase(it);
			} else {
				LOG_TRACE("Server has created an object with a parent, but the parent is not created locally yet", it->server_object_id, it->server_object_parent_id);
				++it;
			}
		}
	}

	// Iterate over map, delete the objects that haven't been visited, as they must have been deleted on the server side
	for (auto it = _server_to_local_map.cbegin(); it != _server_to_local_map.cend(); /* no increment */) {
		if (!it->second.second) {
			auto object_to_delete = it->second.first;
			LOG_DEBUG("Local object hasn't been visited by TurnBasedServerUpdate, scheduling for deletion", it->second.first);
			M2_DEFER(CreateObjectDeleter(object_to_delete));
			it = _server_to_local_map.erase(it); // Erase from map
		} else {
			++it;
		}
	}

	return std::make_pair(
			server_update.shutdown()
			? ServerUpdateStatus::PROCESSED_SHUTDOWN
			: ServerUpdateStatus::PROCESSED,
			_last_processed_server_update->first);
}

void m2::network::TurnBasedRealClientThread::shutdown() {
	locked_shutdown();
}
