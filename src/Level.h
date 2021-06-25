#ifndef LEVEL_H
#define LEVEL_H

#include "Bucket.h"
#include "InsertionList.h"
#include "Box2DWrapper.h"
#include "Pathfinder.h"

typedef struct _Level {
	Bucket objects;
	InsertionList drawList;
	Bucket eventListeners;
	Bucket physics;
	Bucket graphics;
	Bucket terrainGraphics;
	Bucket defenses;
	Bucket offenses;
	Box2DWorld* world;
	Box2DContactListener* contactListener;
	uint64_t cameraId, playerId;
	Array deleteList; // List of Object IDs
	PathfinderMap pathfinderMap;
} Level;

int LevelInit(Level* level);
void LevelDeleteMarkedObjects(Level* level);
void LevelDeinit(Level* level);

int LevelLoadTest(Level* level);
int LevelLoadEditor(Level* level);

#endif
