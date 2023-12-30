#pragma once
#include "Controls.h"
#include "Group.h"
#include "Object.h"
#include "ui/Blueprint.h"
#include "Rational.h"
#include "component/Graphic.h"
#include "Meta.h"
#include <m2g_GroupType.pb.h>
#include <Level.pb.h>
#include <string_view>
#include <array>

namespace m2g {
	/// Name of the game, which is used to discover the resources
	extern const std::string_view game_name;

	extern const int default_game_height_m;

	/// Should the b2World have gravity towards downwards direction
	extern const bool gravity;

	/// Is the world (background, the obstacles) static? If true, pathfinder uses caching.
	extern const bool world_is_static;

	/// Is lightning enabled? Darkens the textures.
	extern const bool lightning;

	/// Z component of the focus position, which has the same XY position as the player
	extern float focus_point_height;
	/// Z component of the xy-plane for camera to mouse ray-casts
	extern const float xy_plane_z_component;

	/// Is the camera also a sound listener
	extern const bool camera_is_listener;

	/// Context
	void* create_context();
	void destroy_context(void* context);

	/// UI
	namespace ui {
		const m2::ui::Blueprint* main_menu();
		const m2::ui::Blueprint* pause_menu();
		const m2::ui::Blueprint* left_hud();
		const m2::ui::Blueprint* right_hud();
	}

	/// Controls
	m2::Key scancode_to_key(SDL_Scancode scancode);
	extern const std::array<SDL_Scancode, static_cast<unsigned>(m2::Key::end)> key_to_scancode;

	/// Called before/after a level is loaded
	void pre_single_player_level_init(const std::string& name, const m2::pb::Level& level);
	void post_single_player_level_init(const std::string& name, const m2::pb::Level& level);
	void pre_multi_player_level_init(const std::string& name, const m2::pb::Level& level);
	void post_multi_player_level_init(const std::string& name, const m2::pb::Level& level);

	/// Maps 0-based client indexes to the object IDs in this game instance
	/// For the server, the first item would contain the ObjectId of the player.
	/// For the client with index 1, the second item would contain the ObjectId of the player.
	extern std::vector<m2::ObjectId> multi_player_object_ids;

	int turn_based_multi_player_turn_holder();

	/// Called after a tile is created
	void post_tile_create(m2::Object& obj, pb::SpriteType sprite_type);

	/// Foreground object loaded
	m2::void_expected init_fg_object(m2::Object& obj);

	/// Create Group for the given type
	m2::Group* create_group(pb::GroupType group_type);
}
