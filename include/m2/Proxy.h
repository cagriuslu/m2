#pragma once
#include "Controls.h"
#include "Group.h"
#include "Object.h"
#include "ui/Blueprint.h"
#include "Rational.h"
#include "component/Graphic.h"
#include "Meta.h"
#include <m2g_GroupType.pb.h>
#include <Network.pb.h>
#include <Level.pb.h>
#include <string_view>
#include <array>

namespace m2 {
	class Proxy {
	   public:
		// TODO make these private, give access to Game, implement getters
		Proxy() = default;
		virtual ~Proxy() = default;

		/// Name of the game, which is used to discover the resources
		const std::string game_name = "<game-name>";

		const int default_game_height_m = 16;

		const std::string default_font_path = "resource/fonts/VT323/VT323-Regular.ttf";
		const int default_font_size = 280; // Also corresponds to font letter height
		const int default_font_letter_width = 112;

		/// Should the world have gravity towards downwards direction
		const bool gravity = false;

		/// Is the world (background, the obstacles) static? If true, pathfinder uses caching.
		const bool world_is_static = true;

		/// Is lightning enabled? Darkens the textures.
		const bool lightning = false;

		/// Z component of the focus position, which has the same XY position as the player
		float focus_point_height = 2.0f;

		/// Z component of the xy-plane for camera to mouse ray-casts
		const float xy_plane_z_component = 0.75f;

		/// Is the camera also a sound listener
		const bool camera_is_listener = false;

		/// Dimming factor of the graphics when dimming is enabled
		const float dimming_factor = 0.333f;

		void load_resources() {}

		// UI
		const m2::ui::Blueprint* main_menu() { return nullptr; }
		const m2::ui::Blueprint* pause_menu() { return nullptr; }
		const m2::ui::Blueprint* left_hud() { return nullptr; }
		const m2::ui::Blueprint* right_hud() { return nullptr; }

		// Controls
		m2::Key scancode_to_key(SDL_Scancode scancode);
		const std::array<SDL_Scancode, static_cast<unsigned>(m2::Key::end)> key_to_scancode = {
		    SDL_SCANCODE_UNKNOWN,
		    SDL_SCANCODE_ESCAPE,
		    SDL_SCANCODE_W,
		    SDL_SCANCODE_S,
		    SDL_SCANCODE_A,
		    SDL_SCANCODE_D,
		    SDL_SCANCODE_SPACE,
		    SDL_SCANCODE_GRAVE,
		    SDL_SCANCODE_RETURN,
		    SDL_SCANCODE_BACKSPACE,
		    SDL_SCANCODE_MINUS,
		    SDL_SCANCODE_EQUALS
		};

		/// Called before/after a single player level is loaded
		void pre_single_player_level_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}
		void post_single_player_level_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}

		/// These functions must be identical between the host and the client, because the levels are expected to be
		/// identical after they are initialized.
		void pre_multi_player_level_client_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}
		void post_multi_player_level_client_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}

		/// After the levels are initialized (identically), host can populate the random parts of the level, after which
		/// second ServerUpdate will be published.
		void multi_player_level_server_populate(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}

		/// Maps 0-based client indexes to the IDs of the objects that represent a client in this game instance.
		/// While loading the level, this vector should be filled with IDs of identical player objects.
		/// Then in post_multi_player_level_client_init, M2_GAME.client_thread().receiver_index() can be queried to
		/// learn the 0-based index of this game instance, and the corresponding object can be assigned to M2_PLAYER.
		/// For the server, the first item would contain the ObjectId of the player.
		/// For the client with index 1, the second item would contain the ObjectId of the player.
		std::vector<m2::ObjectId> multi_player_object_ids;

		/// Return the new turn_holder_index if command is accepted and a ServerUpdate is necessary.
		/// Return -1 if the game ended.
		std::optional<int> handle_client_command(MAYBE int turn_holder_index, MAYBE const m2g::pb::ClientCommand& client_command) { return std::nullopt; }
		void handle_server_command(MAYBE const m2g::pb::ServerCommand& server_command) {}
		void post_server_update(MAYBE const m2::pb::ServerUpdate& server_update) {}

		/// Called after a tile is created
		void post_tile_create(MAYBE m2::Object& obj, MAYBE m2g::pb::SpriteType sprite_type) {}

		/// Load foreground object from a level blueprint
		m2::void_expected init_level_blueprint_fg_object(MAYBE m2::Object& obj) { return {}; }
		/// Load foreground object from a ServerUpdate. The item and resources should NOT be added to the character,
		/// because those will be added automatically after the initialization.
		m2::void_expected init_server_update_fg_object(MAYBE m2::Object& obj, MAYBE const std::vector<m2g::pb::ItemType>& named_item_types, MAYBE const std::vector<m2::pb::Resource>& resources) { return {}; }

		/// Create Group for the given type
		m2::Group* create_group(MAYBE m2g::pb::GroupType group_type) { return nullptr; }
	};
}
