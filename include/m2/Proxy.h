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

		/// Name of the game, which is used to discover the resources
		const std::string game_name = "<game-name>";

		const int default_game_height_m = 16;

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
		void pre_multi_player_level_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}
		void post_multi_player_level_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}

		/// After the levels are initialized (identically), host can populate the random parts of the level, after which
		/// second ServerUpdate will be published.
		void multi_player_level_host_populate(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}

		/// Maps 0-based client indexes to the object IDs in this game instance
		/// For the server, the first item would contain the ObjectId of the player.
		/// For the client with index 1, the second item would contain the ObjectId of the player.
		std::vector<m2::ObjectId> multi_player_object_ids;

		/// Return the new turn_holder_index if command is accepted and a ServerUpdate is necessary.
		std::optional<int> handle_client_command(MAYBE unsigned turn_holder_index, MAYBE const m2g::pb::ClientCommand& client_command) { return std::nullopt; }

		/// Called after a tile is created
		void post_tile_create(MAYBE m2::Object& obj, MAYBE m2g::pb::SpriteType sprite_type) {}

		/// Load foreground object
		m2::void_expected init_fg_object(MAYBE m2::Object& obj) { return {}; }

		/// Create Group for the given type
		m2::Group* create_group(MAYBE m2g::pb::GroupType group_type) { return nullptr; }
	};
}
