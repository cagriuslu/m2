#ifndef LEVEL_H
#define LEVEL_H

#include "Bucket.h"
#include "InsertionList.h"
#include "Box2DWrapper.h"

typedef struct _Level {
	Bucket objects;
	InsertionList drawList;
	Bucket eventListeners;
	Bucket physics;
	Bucket graphics;
	Box2DWorld* world;
	Box2DContactListener* contactListener;
	uint32_t terrainId, cameraId, playerId;
} Level;

int LevelInit(Level* level);
void LevelDeinit(Level* level);

int LevelLoadTest(Level* level);
int LevelLoadEditor(Level* level);











int LevelTestLoad();
int LevelEditorLoad();
void LevelUnload();

#endif
