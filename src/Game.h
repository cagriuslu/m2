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
	// Tile-set related
	int tileWidth;
	const char* textureImageFilePath;
	const char* textureMetaImageFilePath;
	const char* textureMetaFilePath;
	// Window related
	SDL_Window *sdlWindow;
	SDL_Cursor *sdlCursor;
	SDL_Renderer* sdlRenderer;
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
	// Box2D related
	float physicsStepPerSecond;
	float physicsStepPeriod;
	int velocityIterations;
	int positionIterations;
	// Level related
	bool levelLoaded;
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
	// Game related
	Events events;
	unsigned deltaTicks;
	char consoleInput[1024];
} Game;

#define GAME (gCurrentGame)
extern Game* gCurrentGame;

void Game_SetWidthHeight(Game* game, int width, int height);

int Game_Level_Init(Game* game);
int Game_Level_LoadTest(Game* game);
int Game_Level_LoadEditor(Game* game);
void Game_Level_DeleteMarkedObjects(Game* game);
void Game_Level_Term(Game* game);

Vec2F CurrentPointerPositionInWorld();

#endif
