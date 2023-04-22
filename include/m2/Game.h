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

		std::optional<Level> _level;

	public:
		static void create_instance();
		inline static Game& instance() { return *_instance; }
		static void destroy_instance();

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// WINDOW ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		SDL_Window *sdlWindow{};
		SDL_Cursor *sdlCursor{};
		SDL_Renderer *sdlRenderer{};
		SDL_Texture *sdlLightTexture{};
		std::optional<AudioManager> audio_manager;
		uint32_t pixelFormat{};
		SDL_Rect windowRect{};
		SDL_Rect gameRect{};
		SDL_Rect gameAndHudRect{};
		SDL_Rect topEnvelopeRect{};
		SDL_Rect bottomEnvelopeRect{};
		SDL_Rect leftEnvelopeRect{};
		SDL_Rect rightEnvelopeRect{};
		SDL_Rect leftHudRect{};
		SDL_Rect rightHudRect{};
        SDL_Rect console_rect{};
		int game_height_mul_m{16}; // Game height controls the zoom of the game
		int game_height_div_m{1};
		int game_ppm{};
		TTF_Font *ttfFont{};
		bool quit{};

		////////////////////////////////////////////////////////////////////////
		////////////////////////////// RESOURCES ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		std::filesystem::path game_resource_dir;
		std::vector<SpriteSheet> sprite_sheets;
		std::optional<SpriteEffectsSheet> sprite_effects_sheet;
		std::vector<Sprite> _sprites;
		std::vector<m2g::pb::SpriteType> level_editor_background_sprites;
		std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> level_editor_object_sprites;
		std::optional<GlyphsSheet> glyphs_sheet;
		std::optional<ShapesSheet> shapes_sheet;
		std::optional<DynamicSheet> dynamic_sheet;
		std::vector<FullItem> _items;
		std::vector<Animation> animations;
		std::vector<Song> _songs;

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// BOX2D /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		const float phy_period{1.0f / 80.0f};
		const int velocityIterations{8};
		const int positionIterations{3};

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// CONFIG ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		const float max_hearing_distance_m{20.0f};
		const float min_hearing_facing_away{0.1f};

		////////////////////////////////////////////////////////////////////////
		///////////////////////////////// MISC /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		void* context{};
		Events events;
		sdl::ticks_t pause_ticks{};
		unsigned deltaTicks_ms{};
		float deltaTime_s{};
		Vec2f mousePositionWRTGameWorld_m;
		Vec2f mousePositionWRTScreenCenter_m;
        std::vector<std::string> console_output;

		Game();
		~Game();

		// Initialization
		void initialize_context();

		// Level management
		VoidValue load_single_player(const std::variant<FilePath,pb::Level>& level_path_or_blueprint, const std::string& level_name = "");
		VoidValue load_level_editor(const std::string& level_resource_path);
		VoidValue load_pixel_editor(const std::string& image_resource_path, int x_offset, int y_offset);
		inline Level& level() { return *_level; }

		// Accessors
		inline const Sprite& get_sprite(m2g::pb::SpriteType sprite_type) { return _sprites[protobuf::enum_index(sprite_type)]; }
		inline SmartPointer<const Item> get_item(m2g::pb::ItemType item_type) { return make_static<const Item>(&_items[protobuf::enum_index(item_type)]); }
		const Song& get_song(m2g::pb::SongType song_type);

		// Modifiers
		void update_window_dims(int window_width, int window_height);
		inline void add_pause_ticks(sdl::ticks_t ticks) { pause_ticks += ticks; }
		void update_mouse_position();
		void add_deferred_action(const std::function<void(void)>& action);
		void execute_deferred_actions();

		// Helpers
		std::pair<int, int> pixel_scale_mul_div(int sprite_ppm) const;

	private:
		void reset_state();
	};
}

#endif
