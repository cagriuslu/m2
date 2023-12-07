#pragma once
#include "Proxy.h"
#include "Level.h"
#include "Shape.h"
#include "AudioManager.h"
#include "protobuf/LUT.h"
#include "Sprite.h"
#include "m2/Events.h"
#include "DrawList.h"
#include "Song.h"
#include "Animation.h"
#include <m2/Object.h>
#include <m2g_ObjectType.pb.h>
#include <m2/Pool.h>
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include "Meta.h"
#include "Item.h"
#include <filesystem>

#define GAME (m2::Game::instance())
#define LEVEL (GAME.level())

#define GAME_AND_HUD_ASPECT_RATIO_MUL (16)
#define GAME_AND_HUD_ASPECT_RATIO_DIV (9)
#define GAME_ASPECT_RATIO_MUL (5)
#define GAME_ASPECT_RATIO_DIV (4)
#define HUD_ASPECT_RATIO_MUL (GAME_AND_HUD_ASPECT_RATIO_MUL * GAME_ASPECT_RATIO_DIV - GAME_ASPECT_RATIO_MUL * GAME_AND_HUD_ASPECT_RATIO_DIV)
#define HUD_ASPECT_RATIO_DIV (GAME_AND_HUD_ASPECT_RATIO_DIV * GAME_ASPECT_RATIO_DIV * 2)

namespace m2 {
	class Game {
		mutable std::optional<VecF> _mouse_position_world_m;
		mutable std::optional<VecF> _screen_center_to_mouse_position_m; // Doesn't mean much in 2.5D mode

	public: // private
		static Game* _instance;

		struct Dimensions {
			int height_m{20}; // Controls the zoom of the game
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
		SDL_Renderer *renderer{};
		SDL_Texture *light_texture{};
		std::optional<AudioManager> audio_manager;
		uint32_t pixel_format{};
		TTF_Font *font{};
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
		pb::LUT<m2::pb::Item, FullItem> items;
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
		static constexpr float min_hearing_facing_away{0.1f}; // Ratio

		////////////////////////////////////////////////////////////////////////
		///////////////////////////////// MISC /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		void* context{};
		Events events;
		sdl::ticks_t pause_ticks{}; // Ticks spent outside of game
		std::optional<sdl::ticks_t> ui_begin_ticks; // Exists only if there is an ongoing sync UI
        std::vector<std::string> console_output;

		Game();
		~Game();

		// Initialization
		void initialize_context();

		// Level management
		void_expected load_single_player(const std::variant<std::filesystem::path,pb::Level>& level_path_or_blueprint, const std::string& level_name = "");
		void_expected load_level_editor(const std::string& level_resource_path);
		void_expected load_pixel_editor(const std::string& image_resource_path, int x_offset, int y_offset);
		void_expected load_sheet_editor(const std::string& sheet_path);
		Level& level() { return *_level; }

		// Accessors
		const Dimensions& dimensions() const { return _dims; }
		const Sprite& get_sprite(const m2g::pb::SpriteType sprite_type) const { return _sprites[pb::enum_index(sprite_type)]; }
		SmartPointer<const Item> get_item(const m2g::pb::ItemType item_type) const { return make_static<const Item>(&items[item_type]); }
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
		void reset_mouse_position() { _mouse_position_world_m = std::nullopt; _screen_center_to_mouse_position_m = std::nullopt; }
		void recalculate_directional_audio();
		void add_deferred_action(const std::function<void(void)>& action);
		void execute_deferred_actions();

	private:
		void reset_state();
		void recalculate_mouse_position2() const;
	};
}
