#ifndef GAME_H
#define GAME_H

#include <impl/component.hh>
#include "m2/Event.hh"
#include "InsertionList.hh"
#include <m2/Object.h>
#include "m2/Box2D.hh"
#include "m2/Cfg.hh"
#include "Pathfinder.hh"
#include "GameProxy.hh"
#include <m2/pool.hh>
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <b2_world.h>

#define GAME_AND_HUD_ASPECT_RATIO (16.0f / 9.0f)
#define GAME_ASPECT_RATIO (5.0f / 4.0f)
#define HUD_ASPECT_RATIO ((GAME_AND_HUD_ASPECT_RATIO - GAME_ASPECT_RATIO) / 2.0f) // which is 19:72

struct Game {
	////////////////////////////////////////////////////////////////////////
	//////////////////////////////// WINDOW ////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	SDL_Window *sdlWindow;
	SDL_Cursor *sdlCursor;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Texture* sdlTextureMask;
	SDL_Texture* sdlLightTexture;
	uint32_t pixelFormat;
	SDL_Rect windowRect;
	SDL_Rect gameRect;
	SDL_Rect gameAndHudRect;
	SDL_Rect firstEnvelopeRect;
	SDL_Rect secondEnvelopeRect;
	SDL_Rect leftHudRect;
	SDL_Rect rightHudRect;
	float pixelsPerMeter;
	float scale;
	TTF_Font* ttfFont;

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
    m2::pool<m2::Object> objects;
	InsertionList drawList;
    m2::pool<Monitor> monitors;
	m2::pool<Physique> physics;
    m2::pool<Graphic> graphics;
    m2::pool<Graphic> terrainGraphics;
    m2::pool<Light> lights;
    m2::pool<impl::Defense> defenses;
    m2::pool<impl::Offense> offenses;
	b2World* world;
	ContactListener* contactListener;
	ID cameraId, playerId, pointerId;
	std::vector<ID> delete_list;
	PathfinderMap pathfinderMap;
	UIState leftHudUIState, rightHudUIState;

	////////////////////////////////////////////////////////////////////////
	///////////////////////////////// GAME /////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	Events events;
	unsigned deltaTicks_ms;
	float deltaTime_s;
	m2::vec2f mousePositionInWorld_m;
	m2::vec2f mousePositionWRTScreenCenter_m;
	char consoleInput[1024];
	const m2::game_proxy& proxy;

    ~Game();
};

extern Game GAME;

void Game_UpdateWindowDimensions(int width, int height);
void Game_UpdateMousePosition();

M2Err Game_Level_Load(const CfgLevel *cfg);

void Game_DeleteList_Add(ID id);
void Game_DeleteList_DeleteAll();

#endif
