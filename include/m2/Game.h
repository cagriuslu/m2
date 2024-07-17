#pragma once
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <m2/Object.h>
#include <m2/Pool.h>
#include <m2g/Proxy.h>
#include <m2g_ObjectType.pb.h>

#include <filesystem>
#include <functional>
#include <vector>

#include "AudioManager.h"
#include "DrawList.h"
#include "Item.h"
#include "Level.h"
#include "Meta.h"
#include "Proxy.h"
#include "Shape.h"
#include "Song.h"
#include "Sprite.h"
#include "m2/Events.h"
#include "m2/game/Animation.h"
#include "network/ClientThread.h"
#include "network/ServerThread.h"
#include "protobuf/LUT.h"

#define M2_GAME (m2::Game::instance())
#define M2_DEFER(f) (M2_GAME.add_deferred_action(f))
#define M2G_PROXY (M2_GAME._proxy)
#define M2_LEVEL (M2_GAME.level())
#define M2_PLAYER (*M2_LEVEL.player())

#define GAME_AND_HUD_ASPECT_RATIO_MUL (16)
#define GAME_AND_HUD_ASPECT_RATIO_DIV (9)
#define GAME_ASPECT_RATIO_MUL (5)
#define GAME_ASPECT_RATIO_DIV (4)
#define HUD_ASPECT_RATIO_MUL \
	(GAME_AND_HUD_ASPECT_RATIO_MUL * GAME_ASPECT_RATIO_DIV - GAME_ASPECT_RATIO_MUL * GAME_AND_HUD_ASPECT_RATIO_DIV)
#define HUD_ASPECT_RATIO_DIV (GAME_AND_HUD_ASPECT_RATIO_DIV * GAME_ASPECT_RATIO_DIV * 2)

namespace m2 {
	class Game {
		mutable std::optional<VecF> _mouse_position_world_m;
		mutable std::optional<VecF> _screen_center_to_mouse_position_m;  // Doesn't mean much in 2.5D mode

	   public:  // TODO private
		static Game* _instance;
		::m2g::Proxy _proxy{};

		struct Dimensions {
			int height_m{20};  // Controls the zoom of the game
			float width_m{};
			int ppm{};
			RectI window{}, game{}, game_and_hud{};
			RectI top_envelope{}, bottom_envelope{}, left_envelope{}, right_envelope{};
			RectI left_hud{}, right_hud{}, message_box{};
			Dimensions() = default;
			Dimensions(int game_height_m, int window_width, int window_height);
		} _dims;

		////////////////////////////////////////////////////////////////////////
		////////////////////////////// RESOURCES ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		std::vector<Sprite> _sprites;

		std::optional<Level> _level;
		// Client server comes after server thread, thus during shutdown, it'll be killed before the ServerThread.
		// This is important for the server thread to not linger too much.
		std::optional<network::ServerThread> _server_thread;
		std::optional<network::ClientThread> _client_thread;
		bool _server_update_necessary{};
		std::optional<std::set<ObjectId>> _dimming_exceptions;
		float _delta_time_s{};

	   public:
		static void create_instance();
		inline static Game& instance() { return *_instance; }
		static void destroy_instance();

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// WINDOW ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		SDL_Window* window{};
		SDL_Cursor* cursor{};
		SDL_Renderer* renderer{};
		SDL_Texture* light_texture{};
		std::optional<AudioManager> audio_manager;
		uint32_t pixel_format{};
		TTF_Font* font{};
		bool quit{};

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// PATHS /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		std::filesystem::path resource_dir;
		std::filesystem::path levels_dir;

		////////////////////////////////////////////////////////////////////////
		////////////////////////////// RESOURCES ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		std::vector<SpriteSheet> sprite_sheets;
		std::optional<SpriteEffectsSheet> sprite_effects_sheet;
		std::vector<m2g::pb::SpriteType> level_editor_background_sprites;
		std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> object_main_sprites;
		std::optional<ShapesSheet> shapes_sheet;
		std::optional<DynamicSheet> dynamic_sheet;
		pb::LUT<m2::pb::Item, NamedItem> named_items;
		pb::LUT<m2::pb::Animation, Animation> animations;
		pb::LUT<m2::pb::Song, Song> songs;

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// CONFIG ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		static constexpr unsigned phy_frequency{100};
		static constexpr float phy_period{1.0f / static_cast<float>(phy_frequency)};
		static constexpr unsigned phy_period_ticks{1000u / phy_frequency};
		static constexpr int velocity_iterations{8};
		static constexpr int position_iterations{3};
		static constexpr float max_hearing_distance_m{20.0f};
		static constexpr float min_hearing_facing_away{0.1f};  // Ratio

		////////////////////////////////////////////////////////////////////////
		///////////////////////////////// MISC /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		Events events;
		sdl::ticks_t pause_ticks{};  // Ticks spent outside of game
		std::optional<sdl::ticks_t> ui_begin_ticks;  // Exists only if there is an ongoing sync UI
		std::vector<std::string> console_output;

		Game();
		~Game();

		// Proxy management
		Proxy& proxy() { return _proxy; }
		int32_t hash() const { return I(std::hash<std::string>{}(_proxy.game_name)); }
		// Network management
		void_expected host_game(mplayer::Type type, unsigned max_connection_count);
		void_expected join_game(mplayer::Type type, const std::string& addr);
		bool is_server() const { return static_cast<bool>(_server_thread); }
		network::ServerThread& server_thread() { return *_server_thread; }
		network::ClientThread& client_thread() { return *_client_thread; }
		// Level management
		void_expected load_single_player(
		    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint,
		    const std::string& level_name = "");
		void_expected load_multi_player_as_host(
		    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint,
		    const std::string& level_name = "");
		void_expected load_multi_player_as_guest(
		    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint,
		    const std::string& level_name = "");
		void_expected load_level_editor(const std::string& level_resource_path);
		void_expected load_pixel_editor(const std::string& image_resource_path, int x_offset, int y_offset);
		void_expected load_sheet_editor();
		void_expected load_bulk_sheet_editor();
		Level& level() { return *_level; }

		// Accessors
		const Dimensions& dimensions() const { return _dims; }
		const Sprite& get_sprite(const m2g::pb::SpriteType sprite_type) const { return _sprites[pb::enum_index(sprite_type)]; }
		void for_each_sprite(const std::function<bool(m2g::pb::SpriteType, const Sprite&)>& op) const;
		const NamedItem& get_named_item(const m2g::pb::ItemType item_type) const { return named_items[item_type]; }
		void for_each_named_item(const std::function<bool(m2g::pb::ItemType, const NamedItem&)>& op) const;
		const std::optional<std::set<ObjectId>>& dimming_exceptions() const { return _dimming_exceptions; }
		float delta_time_s() const { return _delta_time_s; }
		const VecF& mouse_position_world_m() const;
		const VecF& screen_center_to_mouse_position_m() const;
		VecF pixel_to_2d_world_m(const VecI& pixel_position);
		RectF viewport_to_2d_world_rect_m();

		// Handlers
		void handle_quit_event();
		void handle_window_resize_event();
		void handle_console_event();
		void handle_menu_event();
		void handle_hud_events();
		void execute_pre_step();
		void update_characters();
		void execute_step();
		void execute_post_step();
		void update_sounds();
		void execute_pre_draw();
		void update_hud_contents();
		void clear_back_buffer() const;
		void draw_background();
		void draw_foreground();
		void draw_lights();
		void execute_post_draw();
		void debug_draw();
		void draw_hud();
		void draw_envelopes() const;
		void flip_buffers() const;

		// Modifiers
		void add_pause_ticks(const sdl::ticks_t ticks) { pause_ticks += ticks; }
		void recalculate_dimensions(int window_width, int window_height, int game_height_m = 0);
		void set_zoom(float game_height_multiplier);
		void reset_mouse_position() {
			_mouse_position_world_m = std::nullopt;
			_screen_center_to_mouse_position_m = std::nullopt;
		}
		void recalculate_directional_audio();
		/// Enable dimming mode with exceptions where sprite sheets are dimmed for all objects except the exceptions.
		void enable_dimming_with_exceptions(std::set<ObjectId> exceptions);
		void disable_dimming_with_exceptions();
		void add_deferred_action(const std::function<void(void)>& action);
		void execute_deferred_actions();

	   private:
		void reset_state();
		void recalculate_mouse_position2() const;
	};
}  // namespace m2
