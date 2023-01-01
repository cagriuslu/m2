#ifndef GAME_H
#define GAME_H

#include <m2g/Proxy.h>
#include "Level.h"
#include "Group.h"
#include "Shape.h"
#include "Sprite.h"
#include "m2/Events.h"
#include "DrawList.h"
#include "Animation.h"
#include <m2/Object.h>
#include <ObjectType.pb.h>
#include "Pathfinder.hh"
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

#define GAME (::m2::Game::instance())
#define CGAME (::m2::Game::const_instance())

#define GAME_AND_HUD_ASPECT_RATIO_MUL (16)
#define GAME_AND_HUD_ASPECT_RATIO_DIV (9)
#define GAME_ASPECT_RATIO_MUL (5)
#define GAME_ASPECT_RATIO_DIV (4)
#define HUD_ASPECT_RATIO_MUL (GAME_AND_HUD_ASPECT_RATIO_MUL * GAME_ASPECT_RATIO_DIV - GAME_ASPECT_RATIO_MUL * GAME_AND_HUD_ASPECT_RATIO_DIV)
#define HUD_ASPECT_RATIO_DIV (GAME_AND_HUD_ASPECT_RATIO_DIV * GAME_ASPECT_RATIO_DIV * 2)

namespace m2 {
	struct Game {
		static Game* _instance;

		std::optional<Level> level;

	public:
		static void create_instance();
		inline static const Game& const_instance() { return *_instance; }
		inline static Game& instance() { return *_instance; }
		static void destroy_instance();

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// WINDOW ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		SDL_Window *sdlWindow{};
		SDL_Cursor *sdlCursor{};
		SDL_Renderer *sdlRenderer{};
		SDL_Texture *sdlLightTexture{};
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

		////////////////////////////////////////////////////////////////////////
		////////////////////////////// RESOURCES ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		std::vector<SpriteSheet> sprite_sheets;
		std::optional<SpriteEffectsSheet> sprite_effects_sheet;
		std::vector<Sprite> sprites;
		std::vector<m2g::pb::SpriteType> editor_background_sprites;
		std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> editor_object_sprites;
		std::optional<GlyphsSheet> glyphs_sheet;
		std::optional<ShapesSheet> shapes_sheet;
		std::optional<DynamicSheet> dynamic_sheet;
		std::vector<Item> _items;
		std::vector<Animation> animations;

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// BOX2D /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		const float phy_period{1.0f / 80.0f};
		const int velocityIterations{8};
		const int positionIterations{3};

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// LEVEL /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		// Objects are not meant to be iterated over, as it holds all types of objects. If a certain type of object needs to
		// be iterated over, create a component, attach the component to an object, put component in separate pool, and
		// iterate over that pool.
		// Another reason to put a component inside a Pool: if the type of object that is using that component is
		// created/destroyed very rapidly.
		Pool<Object> objects;
		std::unordered_map<GroupId, std::unique_ptr<Group>, GroupId::Hash> groups;
		DrawList draw_list;
		Pool<Physique> physics;
		Pool<Graphic> graphics;
		Pool<Graphic> terrainGraphics;
		Pool<Light> lights;
		Pool<CharacterVariant> characters;
		b2World *world{};
        box2d::ContactListener* contactListener{};
		Id cameraId{}, playerId{}, pointerId{};
		std::optional<Pathfinder> pathfinder;
        std::optional<ui::State> leftHudUIState, rightHudUIState;

		////////////////////////////////////////////////////////////////////////
		///////////////////////////////// GAME /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		Events events;
		unsigned deltaTicks_ms{};
		float deltaTime_s{};
		Vec2f mousePositionWRTGameWorld_m;
		Vec2f mousePositionWRTScreenCenter_m;
        std::vector<std::string> console_output;

		Game();
		~Game();

		// Level management
		VoidValue load_level(const std::string& level_resource_path);
		VoidValue load_level(const pb::Level& lb);
		VoidValue load_editor(const std::string& level_resource_path);
		void unload_level();

		// Accessors
		Object* player();
		const Item& get_item(m2g::pb::ItemType item_type);

		// Modifiers
		void update_window_dims(int window_width, int window_height);
		void update_mouse_position();
		void add_deferred_action(const std::function<void(void)>& action);
		void execute_deferred_actions();

		// Helpers
		std::pair<int, int> pixel_scale_mul_div(int sprite_ppm) const;

	private:
		VoidValue internal_load_level(const pb::Level& level_blueprint);
	};
}

#endif
