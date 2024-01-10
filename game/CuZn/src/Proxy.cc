#include <m2/Proxy.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Game.h>
#include <m2/multi_player/State.h>

const std::string_view m2g::game_name = "CuZn";
const int m2g::default_game_height_m = 16;
const bool m2g::gravity = false;
const bool m2g::world_is_static = true;
const bool m2g::lightning = false;
float m2g::focus_point_height = 2.0f;
const float m2g::xy_plane_z_component = 0.75f;
const bool m2g::camera_is_listener = false;

void* m2g::create_context() {
	return nullptr;
}
void m2g::destroy_context(MAYBE void* context) {
}

m2::Key m2g::scancode_to_key(SDL_Scancode scancode) {
	using namespace m2;
	switch (scancode) {
		case SDL_SCANCODE_ESCAPE:
			return Key::MENU;
		case SDL_SCANCODE_W:
			return Key::UP;
		case SDL_SCANCODE_S:
			return Key::DOWN;
		case SDL_SCANCODE_A:
			return Key::LEFT;
		case SDL_SCANCODE_D:
			return Key::RIGHT;
		case SDL_SCANCODE_SPACE:
			return Key::DASH;
		case SDL_SCANCODE_GRAVE:
			return Key::CONSOLE;
		case SDL_SCANCODE_RETURN:
			return Key::ENTER;
		case SDL_SCANCODE_BACKSPACE:
			return Key::BACKSPACE;
		default:
			return Key::UNKNOWN;
	}
}

const std::array<SDL_Scancode, static_cast<unsigned>(m2::Key::end)> m2g::key_to_scancode = {
		SDL_SCANCODE_UNKNOWN,
		SDL_SCANCODE_ESCAPE,
		SDL_SCANCODE_W,
		SDL_SCANCODE_S,
		SDL_SCANCODE_A,
		SDL_SCANCODE_D,
		SDL_SCANCODE_SPACE,
		SDL_SCANCODE_GRAVE,
		SDL_SCANCODE_RETURN,
		SDL_SCANCODE_BACKSPACE
};

void m2g::pre_single_player_level_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}
void m2g::post_single_player_level_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}
void m2g::pre_multi_player_level_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}
void m2g::post_multi_player_level_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}
void m2g::multi_player_level_host_populate() {

}

std::vector<m2::ObjectId> m2g::multi_player_object_ids;

std::optional<int> m2g::handle_client_command(unsigned turn_holder_index, const m2g::pb::ClientCommand& client_command) {
	LOG_INFO("Received command from client", turn_holder_index);

	// Increment turn holder
	return (turn_holder_index + 1) % GAME.server_thread().client_count();
}

void m2g::post_tile_create(MAYBE m2::Object& obj, MAYBE pb::SpriteType sprite_type) {}

m2::void_expected m2g::init_fg_object(m2::Object& obj) {
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
		multi_player_object_ids.emplace_back(obj.id());

		// If host
		if (GAME.is_server() && LEVEL.player_id == 0) {
			// Save player ID
			LEVEL.player_id = obj.id();
		}
		// At this point, the ServerUpdate is not yet processed
		else if (GAME.client_thread().peek_unprocessed_server_update()->receiver_index() == (multi_player_object_ids.size() - 1)) {
			// Save player ID
			LEVEL.player_id = obj.id();
		}
	}

	return {};
}

m2::Group* m2g::create_group(MAYBE pb::GroupType group_type) {
	return nullptr;
}
