#pragma once
#include "AudioManager.h"
#include "Cache.h"
#include "DrawList.h"
#include "Events.h"
#include "Item.h"
#include "Level.h"
#include "Meta.h"
#include "Object.h"
#include "Pool.h"
#include "Proxy.h"
#include "Shape.h"
#include "Song.h"
#include "Sprite.h"
#include "game/Animation.h"
#include "network/HostClientThread.h"
#include "network/RealClientThread.h"
#include "network/BotClientThread.h"
#include "network/ServerThread.h"
#include "protobuf/LUT.h"
#include <m2g_ObjectType.pb.h>
#include <m2g/Proxy.h>
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <filesystem>
#include <functional>
#include <vector>
#include "math/Hash.h"

#define M2_GAME (m2::Game::instance())
#define M2_DEFER(f) (M2_GAME.add_deferred_action(f))
#define M2G_PROXY (M2_GAME.proxy())
#define M2_LEVEL (M2_GAME.level())
#define M2_PLAYER (*M2_LEVEL.player())

namespace m2 {
	using ServerThreads = std::pair<network::ServerThread, network::HostClientThread>;
	using BotAndIndexThread = std::pair<network::BotClientThread,int>;

	class Game {
	public:
		struct Dimensions {
			int height_m{20};  // Controls the zoom of the game
			float width_m{};
			int ppm{};
			RectI window{}, game{}, game_and_hud{};
			RectI top_envelope{}, bottom_envelope{}, left_envelope{}, right_envelope{};
			RectI left_hud{}, right_hud{}, message_box{};
			Dimensions() = default;
			Dimensions(int game_height_m, int window_width, int window_height, int game_aspect_ratio_mul, int game_aspect_ratio_div);
			float hud_width_to_game_and_hud_width_ratio() const;
			float game_width_to_game_and_hud_width_ration() const;
		};

	private:
		static Game* _instance;
		::m2g::Proxy _proxy{};
		Dimensions _dims;

		mutable std::optional<VecF> _mouse_position_world_m;
		mutable std::optional<VecF> _screen_center_to_mouse_position_m;  // Doesn't mean much in 2.5D mode

		// Client server comes after server thread, thus during shutdown, it'll be killed before the ServerThread.
		// This is important for the server thread to not linger too much.
		std::variant<std::monostate, ServerThreads, network::RealClientThread> _multi_player_threads;
		std::list<BotAndIndexThread> _bot_threads; // thread,receiver_index pairs (receiver_index is initially -1)
		bool _server_update_necessary{}, _server_update_with_shutdown{};

		////////////////////////////////////////////////////////////////////////
		////////////////////////////// RESOURCES ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		Rational _font_letter_width_to_height_ratio; // letter w/h
		std::vector<Sprite> _sprites;

	public:  // TODO private
		std::optional<Level> _level;
		float _delta_time_s{};

	   public:
		static void create_instance();
		static Game& instance() { return *_instance; }
		static void destroy_instance();

		::m2g::Proxy& proxy() { return _proxy; }

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
		const Rational& font_letter_width_to_height_ratio() const { return _font_letter_width_to_height_ratio; }

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

		int32_t hash() const { return ihash(_proxy.game_identifier); }
		/// For server
		void_expected host_game(mplayer::Type type, unsigned max_connection_count);
		/// For client
		void_expected join_game(mplayer::Type type, const std::string& addr);
		/// For client
		void leave_game();
		/// For server
		bool add_bot();
		/// For server
		network::BotClientThread& find_bot(int receiver_index);
		bool is_server() const { return std::holds_alternative<ServerThreads>(_multi_player_threads); }
		bool is_real_client() const { return std::holds_alternative<network::RealClientThread>(_multi_player_threads); }
		network::ServerThread& server_thread() { return std::get<ServerThreads>(_multi_player_threads).first; }
		network::HostClientThread& host_client_thread() { return std::get<ServerThreads>(_multi_player_threads).second; }
		network::RealClientThread& real_client_thread() { return std::get<network::RealClientThread>(_multi_player_threads); }
		int total_player_count();
		int self_index();
		int turn_holder_index();
		bool is_our_turn();
		void queue_client_command(const m2g::pb::ClientCommand& cmd);
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
		bool has_level() const { return static_cast<bool>(_level); }
		Level& level() { return *_level; }

		// Accessors
		const Dimensions& dimensions() const { return _dims; }
		const Sprite& get_sprite(const m2g::pb::SpriteType sprite_type) const { return _sprites[pb::enum_index(sprite_type)]; }
		void for_each_sprite(const std::function<bool(m2g::pb::SpriteType, const Sprite&)>& op) const;
		const NamedItem& get_named_item(const m2g::pb::ItemType item_type) const { return named_items[item_type]; }
		void for_each_named_item(const std::function<bool(m2g::pb::ItemType, const NamedItem&)>& op) const;
		float delta_time_s() const { return _delta_time_s; }
		const VecF& mouse_position_world_m() const;
		const VecF& screen_center_to_mouse_position_m() const;
		VecF pixel_to_2d_world_m(const VecI& pixel_position);
		RectF viewport_to_2d_world_rect_m();
		sdl::TextureUniquePtr draw_game_to_texture(m2::VecF camera_position);

		// Handlers
		void handle_quit_event();
		void handle_window_resize_event();
		void handle_console_event();
		void handle_menu_event();
		void handle_hud_events();
		void handle_network_events();
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
		void reset_mouse_position() { _mouse_position_world_m = std::nullopt; _screen_center_to_mouse_position_m = std::nullopt; }
		void recalculate_directional_audio();

		void add_deferred_action(const std::function<void(void)>& action);
		void execute_deferred_actions();

	   private:
		void reset_state();
		void recalculate_mouse_position2() const;
	};
}  // namespace m2
