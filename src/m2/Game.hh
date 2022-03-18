#ifndef GAME_H
#define GAME_H

#include "m2/Event.hh"
#include "Pool.hh"
#include "InsertionList.hh"
#include "Object.hh"
#include "m2/Box2D.hh"
#include "m2/Cfg.hh"
#include "Pathfinder.hh"
#include "SpatialMap.hh"
#include "GameProxy.hh"
#include <SDL.h>
#include <SDL_ttf.h>

#define GAME_AND_HUD_ASPECT_RATIO (16.0f / 9.0f)
#define GAME_ASPECT_RATIO (5.0f / 4.0f)
#define HUD_ASPECT_RATIO ((GAME_AND_HUD_ASPECT_RATIO - GAME_ASPECT_RATIO) / 2.0f) // which is 19:72

typedef struct _Game {
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
	Pool objects;
	InsertionList drawList;
	Pool monitors;
	Pool physics;
	Pool graphics;
	Pool terrainGraphics;
	Pool lights;
	Pool defenses;
	Pool offenses;
	Box2DWorld* world;
	Box2DContactListener* contactListener;
	ID cameraId, playerId, pointerId;
	Array deleteList; // List of Object IDs
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
	GameProxy proxy;
} Game;

#define GAME (Game_GetCurrent())
Game* Game_GetCurrent();

void Game_UpdateWindowDimensions(int width, int height);
void Game_UpdateMousePosition();

M2Err Game_Level_Load(const CfgLevel *cfg);

Object* Game_FindObjectById(ID id);
void Game_DeleteList_Add(ID id);
void Game_DeleteList_DeleteAll();

#endif