#ifndef GAME_H
#define GAME_H

#include <m2g/Proxy.h>
#include <m2g/SpriteId.h>
#include "Level.h"
#include "Group.h"
#include "Sprite.h"
#include <GroupBlueprint.pb.h>
#include <m2g/component/Defense.h>
#include <m2g/component/Offense.h>
#include "m2/Events.h"
#include "DrawList.h"
#include <m2/Object.h>
#include "Pathfinder.hh"
#include "m2/Ui.h"
#include "component/Monitor.h"
#include "component/Physique.h"
#include "component/Graphic.h"
#include <m2/box2d/ContactListener.h>
#include <m2/Pool.hh>
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <b2_world.h>
#include <Sprite.pb.h>
#include <sstream>
#include "Value.h"
#include <unordered_map>
#include "M2.h"
#include <filesystem>

#define GAME_AND_HUD_ASPECT_RATIO (16.0f / 9.0f)
#define GAME_ASPECT_RATIO (5.0f / 4.0f)
#define HUD_ASPECT_RATIO ((GAME_AND_HUD_ASPECT_RATIO - GAME_ASPECT_RATIO) / 2.0f) // which is 19:72

namespace m2 {
	struct Game {
		std::optional<Level> level;

	public:
		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// WINDOW ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		SDL_Window *sdlWindow{};
		SDL_Cursor *sdlCursor{};
		SDL_Renderer *sdlRenderer{};
		SDL_Texture *sdlTexture{};
		SDL_Texture *sdlTextureMask{};
		SDL_Texture *sdlLightTexture{};
		uint32_t pixelFormat{};
		SDL_Rect windowRect{};
		SDL_Rect gameRect{};
		SDL_Rect gameAndHudRect{};
		SDL_Rect firstEnvelopeRect{};
		SDL_Rect secondEnvelopeRect{};
		SDL_Rect leftHudRect{};
		SDL_Rect rightHudRect{};
        SDL_Rect console_rect{};
		float game_height_m{16.0f}; // This controls the zoom of the game
		float game_ppm{};
		TTF_Font *ttfFont{};

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// SPRITES ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		SpriteSheetKeyToSpriteSheetMap sprite_sheets;
		SpriteKeyToSpriteMap sprite_key_to_sprite_map;
		SpriteIdToSpriteLut sprite_id_lut;
		const Sprite& lookup_sprite(m2g::SpriteId) const;
		SpriteKeyToSpriteIdMap sprite_key_to_id_map;
		EditorPaletteSpriteKeys editor_bg_sprites;
		EditorPaletteSpriteKeys editor_fg_sprites;

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// BOX2D /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		const float physicsStep_s{1.0f / 80.0f};
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
		std::unordered_map<GroupId, std::unique_ptr<Group>, GroupIdHasher> groups;
		DrawList draw_list;
		Pool<comp::Monitor> monitors;
		Pool<comp::Physique> physics;
		Pool<comp::Graphic> graphics;
		Pool<comp::Graphic> terrainGraphics;
		Pool<comp::Light> lights;
		Pool<m2g::comp::Defense> defenses;
		Pool<m2g::comp::Offense> offenses;
		b2World *world{};
        box2d::ContactListener* contactListener{};
		Id cameraId{}, playerId{}, pointerId{};
		PathfinderMap pathfinderMap;
        std::optional<ui::State> leftHudUIState, rightHudUIState;

		////////////////////////////////////////////////////////////////////////
		///////////////////////////////// GAME /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		Events events;
		unsigned deltaTicks_ms{};
		float deltaTime_s{};
		bool is_phy_stepping{};
		Vec2f mousePositionWRTGameWorld_m;
		Vec2f mousePositionWRTScreenCenter_m;
        std::vector<std::string> console_output;

		Game();
		~Game();

		// Level management
		VoidValue load_level(const std::string& level_resource_path);
		VoidValue load_editor(const std::string& level_resource_path);
		void unload_level();

		// Accessors
		Object* player();

		// Modifiers
		void update_window_dims(int width, int height);
		void update_mouse_position();
		void add_deferred_action(const std::function<void(void)>& action);
		void execute_deferred_actions();

		// Helpers
		float pixel_scale(float sprite_ppm) const;
	};
}

extern m2::Game* g_game;
#define GAME (*g_game)

#endif
