#ifndef GAME_H
#define GAME_H

#include "Event.h"
#include "Pool.h"
#include "InsertionList.h"
#include "Box2D.h"
#include "Hud.h"
#include "Pathfinder.h"
#include "SpatialMap.h"
#include <SDL.h>
#include <SDL_ttf.h>

#define GAME_AND_HUD_ASPECT_RATIO (16.0f / 9.0f)
#define GAME_ASPECT_RATIO (5.0f / 4.0f)
#define HUD_ASPECT_RATIO ((GAME_AND_HUD_ASPECT_RATIO - GAME_ASPECT_RATIO) / 2.0f)

typedef enum _LevelType {
	LEVEL_TYPE_INVALID = 0,
	LEVEL_TYPE_SINGLE_PLAYER,
	LEVEL_TYPE_LEVEL_EDITOR,
} LevelType;

typedef struct _Game {
	////////////////////////////////////////////////////////////////////////
	/////////////////////////////// TILESET ////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	int tileWidth;
	const char* textureImageFilePath;
	const char* textureMetaImageFilePath;
	const char* textureMetaFilePath;

	////////////////////////////////////////////////////////////////////////
	//////////////////////////////// WINDOW ////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	SDL_Window *sdlWindow;
	SDL_Cursor *sdlCursor;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	uint32_t pixelFormat;
	int windowWidth;
	int windowHeight;
	int gameWidth;
	int gameHeight;
	int gameAndHudWidth;
	int gameAndHudHeight;
	SDL_Rect firstEnvelopeRect;
	SDL_Rect secondEnvelopeRect;
	SDL_Rect leftHudRect;
	SDL_Rect rightHudRect;
	float pixelsPerMeter;
	TTF_Font* ttfFont;

	////////////////////////////////////////////////////////////////////////
	//////////////////////////////// BOX2D /////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	float physicsStepPerSecond;
	float physicsStepPeriod;
	int velocityIterations;
	int positionIterations;

	////////////////////////////////////////////////////////////////////////
	//////////////////////////////// LEVEL /////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	bool levelLoaded;
	// Objects are not meant to be iterated over, as it holds all types of objects
	// If a certain type of object needs to be iterated over, create a component,
	// attach the component to an object, put component in separate pool,
	// and iterate over that pool.
	Pool objects;
	InsertionList drawList;
	Pool eventListeners;
	Pool physics;
	Pool graphics;
	Pool terrainGraphics;
	Pool defenses;
	Pool offenses;
	Box2DWorld* world;
	Box2DContactListener* contactListener;
	LevelType levelType;
	ID cameraId, playerId;
	Array deleteList; // List of Object IDs
	PathfinderMap pathfinderMap;
	Hud hud;

	////////////////////////////////////////////////////////////////////////
	///////////////////////////////// GAME /////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	Events events;
	unsigned deltaTicks;
	char consoleInput[1024];
} Game;

#define GAME (gCurrentGame)
extern Game* gCurrentGame;

void Game_UpdateWindowDimensions(int width, int height);

int Game_Level_Init();
int Game_Level_LoadTest();
int Game_Level_LoadEditor();
void Game_Level_DeleteMarkedObjects();
void Game_Level_Term();

Vec2F CurrentPointerPositionInWorld();

#endif
