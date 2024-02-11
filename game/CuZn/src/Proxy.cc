#include <m2/Proxy.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Game.h>
#include <m2/multi_player/State.h>

void m2g::Proxy::multi_player_level_host_populate(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {
	auto client_count = GAME.server_thread().client_count();
	// TODO
}

std::optional<int> m2g::Proxy::handle_client_command(unsigned turn_holder_index, MAYBE const m2g::pb::ClientCommand& client_command) {
	LOG_INFO("Received command from client", turn_holder_index);

	// Increment turn holder
	return (turn_holder_index + 1) % GAME.server_thread().client_count();
}

m2::void_expected m2g::Proxy::init_fg_object(m2::Object& obj) {
	m2::void_expected init_result;
	switch (obj.object_type()) {
		case pb::HUMAN_PLAYER:
			init_result = cuzn::init_human_player(obj);
			break;
		default:
			return m2::make_unexpected("Invalid object type");
	}
	m2_reflect_failure(init_result);

	if (obj.object_type() == m2g::pb::ObjectType::HUMAN_PLAYER) {
		PROXY.multi_player_object_ids.emplace_back(obj.id());

		// If host
		if (GAME.is_server() && LEVEL.player_id == 0) {
			// Save player ID
			LEVEL.player_id = obj.id();
		}
		// At this point, the ServerUpdate is not yet processed
		else if (GAME.client_thread().peek_unprocessed_server_update()->receiver_index() == (PROXY.multi_player_object_ids.size() - 1)) {
			// Save player ID
			LEVEL.player_id = obj.id();
		}
	}

	return {};
}
