#ifndef GAME_H
#define GAME_H

#include "Proxy.h"
#include "Level.h"
#include "Group.h"
#include "Shape.h"
#include "AudioManager.h"
#include "Sprite.h"
#include "m2/Events.h"
#include "DrawList.h"
#include "Song.h"
#include "Animation.h"
#include <m2/Object.h>
#include <ObjectType.pb.h>
#include "Rational.h"
#include "Pathfinder.h"
#include "m2/Ui.h"
#include "component/Physique.h"
#include "component/Graphic.h"
#include <m2/box2d/ContactListener.h>
#include <m2/Pool.hh>
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <box2d/b2_world.h>
#include <Sprite.pb.h>
#include <sstream>
#include "Value.h"
#include "M2.h"
#include "Item.h"
#include <filesystem>
#include <unordered_map>

#define GAME (m2::Game::instance())
#define LEVEL (GAME.level())

#define GAME_AND_HUD_ASPECT_RATIO_MUL (16)
#define GAME_AND_HUD_ASPECT_RATIO_DIV (9)
#define GAME_ASPECT_RATIO_MUL (5)
#define GAME_ASPECT_RATIO_DIV (4)
#define HUD_ASPECT_RATIO_MUL (GAME_AND_HUD_ASPECT_RATIO_MUL * GAME_ASPECT_RATIO_DIV - GAME_ASPECT_RATIO_MUL * GAME_AND_HUD_ASPECT_RATIO_DIV)
#define HUD_ASPECT_RATIO_DIV (GAME_AND_HUD_ASPECT_RATIO_DIV * GAME_ASPECT_RATIO_DIV * 2)

namespace m2 {
	struct Game {
		static Game* _instance;

		struct Dimensions {
			Rational height_m{20}; // Controls the zoom of the game
			int ppm{};
			SDL_Rect window{}, game{}, game_and_hud{};
			SDL_Rect top_envelope{}, bottom_envelope{}, left_envelope{}, right_envelope{};
			SDL_Rect left_hud{}, right_hud{}, console{};
			Dimensions() = default;
			Dimensions(const Rational& game_height_m, int window_width, int window_height);
		} _dims;

		////////////////////////////////////////////////////////////////////////
		////////////////////////////// RESOURCES ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		std::vector<Sprite> _sprites;
		std::vector<FullItem> _items;
		std::vector<Song> _songs;

		std::optional<Level> _level;
		float _delta_time_s{};
		Vec2f _mouse_position_world_m;
		Vec2f _screen_center_to_mouse_position_m;

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
		////////////////////////////// RESOURCES ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		std::filesystem::path game_resource_dir;
		std::vector<SpriteSheet> sprite_sheets;
		std::optional<SpriteEffectsSheet> sprite_effects_sheet;
		std::vector<m2g::pb::SpriteType> level_editor_background_sprites;
		std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> level_editor_object_sprites;
		std::optional<GlyphsSheet> glyphs_sheet;
		std::optional<ShapesSheet> shapes_sheet;
		std::optional<DynamicSheet> dynamic_sheet;
		std::vector<Animation> animations;

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// CONFIG ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		static constexpr float phy_period{1.0f / 80.0f};
		static constexpr int velocity_iterations{8};
		static constexpr int position_iterations{3};
		static constexpr float max_hearing_distance_m{20.0f};
		static constexpr float min_hearing_facing_away{0.1f}; // Ratio

		////////////////////////////////////////////////////////////////////////
		///////////////////////////////// MISC /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		void* context{};
		Events events;
		sdl::ticks_t pause_ticks{};
        std::vector<std::string> console_output;

		Game();
		~Game();

		// Initialization
		void initialize_context();

		// Level management
		VoidValue load_single_player(const std::variant<std::filesystem::path,pb::Level>& level_path_or_blueprint, const std::string& level_name = "");
		VoidValue load_level_editor(const std::string& level_resource_path);
		VoidValue load_pixel_editor(const std::string& image_resource_path, int x_offset, int y_offset);
		inline Level& level() { return *_level; }

		// Accessors
		inline const Dimensions& dimensions() const { return _dims; }
		inline const Sprite& get_sprite(m2g::pb::SpriteType sprite_type) { return _sprites[protobuf::enum_index(sprite_type)]; }
		inline SmartPointer<const Item> get_item(m2g::pb::ItemType item_type) { return make_static<const Item>(&_items[protobuf::enum_index(item_type)]); }
		const Song& get_song(m2g::pb::SongType song_type);
		inline float delta_time_s() const { return _delta_time_s; }
		inline const Vec2f& mouse_position_world_m() const { return _mouse_position_world_m; }
		inline const Vec2f& screen_center_to_mouse_position_m() const { return _screen_center_to_mouse_position_m; }

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
		void clear_back_buffer();
		void draw_background();
		void draw_foreground();
		void draw_lights();
		void draw_background_effects();
		void draw_foreground_effects();
		void execute_post_draw();
		void draw_debug_shapes();
		void draw_hud();
		void draw_envelopes();
		void flip_buffers();

		// Modifiers
		inline void add_pause_ticks(sdl::ticks_t ticks) { pause_ticks += ticks; }
		void recalculate_dimensions(int window_width, int window_height, const Rational& game_height = {});
		void recalculate_mouse_position();
		void recalculate_directional_audio();
		void add_deferred_action(const std::function<void(void)>& action);
		void execute_deferred_actions();

	private:
		void reset_state();
	};
}

#endif
