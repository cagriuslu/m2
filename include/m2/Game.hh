#ifndef GAME_H
#define GAME_H

#include <m2g/component/Defense.h>
#include <m2g/component/Offense.h>
#include "m2/Event.hh"
#include "DrawList.h"
#include <m2/Object.h>
#include <m2/LevelBlueprint.h>
#include "Pathfinder.hh"
#include "ui/UI.h"
#include "component/Monitor.h"
#include "component/Physique.h"
#include "component/Graphic.h"
#include <m2/box2d/ContactListener.h>
#include <m2/Pool.hh>
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <b2_world.h>
#include <sstream>

#define GAME_AND_HUD_ASPECT_RATIO (16.0f / 9.0f)
#define GAME_ASPECT_RATIO (5.0f / 4.0f)
#define HUD_ASPECT_RATIO ((GAME_AND_HUD_ASPECT_RATIO - GAME_ASPECT_RATIO) / 2.0f) // which is 19:72

namespace m2 {
	struct Game {
		struct Level {
			std::vector<std::function<void(void)>> deferred_actions;
		} level;

	public:
		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// WINDOW ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		SDL_Window *sdlWindow;
		SDL_Cursor *sdlCursor;
		SDL_Renderer *sdlRenderer;
		SDL_Texture *sdlTexture;
		SDL_Texture *sdlTextureMask;
		SDL_Texture *sdlLightTexture;
		uint32_t pixelFormat;
		SDL_Rect windowRect;
		SDL_Rect gameRect;
		SDL_Rect gameAndHudRect;
		SDL_Rect firstEnvelopeRect;
		SDL_Rect secondEnvelopeRect;
		SDL_Rect leftHudRect;
		SDL_Rect rightHudRect;
        SDL_Rect console_rect;
		float pixelsPerMeter;
		float scale;
		TTF_Font *ttfFont;

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// BOX2D /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		const float physicsStep_s;
		const int velocityIterations;
		const int positionIterations;

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// LEVEL /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		bool levelLoaded;
		// Objects are not meant to be iterated over, as it holds all types of objects. If a certain type of object needs to
		// be iterated over, create a component, attach the component to an object, put component in separate pool, and
		// iterate over that pool.
		// Another reason to put a component inside a Pool: if the type of object that is using that component is
		// created/destroyed very rapidly.
		m2::Pool<m2::Object> objects;
		m2::DrawList draw_list;
		m2::Pool<comp::Monitor> monitors;
		m2::Pool<comp::Physique> physics;
		m2::Pool<comp::Graphic> graphics;
		m2::Pool<comp::Graphic> terrainGraphics;
		m2::Pool<comp::Light> lights;
		m2::Pool<m2g::comp::Defense> defenses;
		m2::Pool<m2g::comp::Offense> offenses;
		b2World *world;
        m2::box2d::ContactListener* contactListener;
		ID cameraId, playerId, pointerId;
		PathfinderMap pathfinderMap;
        m2::ui::UIState leftHudUIState, rightHudUIState;

		////////////////////////////////////////////////////////////////////////
		///////////////////////////////// GAME /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		Events events;
		unsigned deltaTicks_ms;
		float deltaTime_s;
		m2::Vec2f mousePositionInWorld_m;
		m2::Vec2f mousePositionWRTScreenCenter_m;
        std::vector<std::string> console_output;

		~Game();

		void add_deferred_action(const std::function<void(void)>& action);
		void execute_deferred_actions();

        static void dynamic_assert();
	};
}

extern m2::Game GAME;

void Game_UpdateWindowDimensions(int width, int height);
void Game_UpdateMousePosition();

M2Err Game_Level_Load(const m2::LevelBlueprint* blueprint);

#endif
