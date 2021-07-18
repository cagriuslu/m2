#ifndef LEVEL_H
#define LEVEL_H

#include "Bucket.h"
#include "InsertionList.h"
#include "Box2D.h"
#include "Pathfinder.h"
#include "SpatialMap.h"

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
	ID cameraId, playerId;
	Array deleteList; // List of Object IDs
	PathfinderMap pathfinderMap;
	Bucket lightSources;
	SpatialMap lightSourceSpatialMap;
	Bucket prePhysicsStopwatches; // holds Arrays of unsigned's
} Level;

int Level_Init(Level* level);
void Level_DeleteMarkedObjects(Level* level);
void Level_Term(Level* level);

int Level_LoadTest(Level* level);
int Level_LoadEditor(Level* level);

#endif
