#ifndef LEVEL_H
#define LEVEL_H

#include "Pool.h"
#include "InsertionList.h"
#include "Box2D.h"
#include "Hud.h"
#include "Pathfinder.h"
#include "SpatialMap.h"

typedef enum _LevelType {
	LEVEL_TYPE_INVALID = 0,
	LEVEL_TYPE_SINGLE_PLAYER,
	LEVEL_TYPE_LEVEL_EDITOR,
} LevelType;

typedef struct _Level {
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
	// Not used
	Pool lightSources;
	SpatialMap lightSourceSpatialMap;
} Level;

int Level_Init(Level* level);
void Level_DeleteMarkedObjects(Level* level);
void Level_Term(Level* level);

int Level_LoadTest(Level* level);
int Level_LoadEditor(Level* level);

#endif
