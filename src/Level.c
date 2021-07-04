#include "Level.h"
#include "Object.h"
#include "Component.h"
#include "Box2D.h"
#include "TerrainLoader.h"
#include <stdio.h>
#include <string.h>

int LevelInit(Level* level) {
	memset(level, 0, sizeof(Level));
	PROPAGATE_ERROR(Bucket_Init(&level->objects, sizeof(Object)));
	PROPAGATE_ERROR(InsertionListInit(&level->drawList, UINT16_MAX + 1, GraphicsComponentYComparatorCB));
	PROPAGATE_ERROR(Bucket_Init(&level->eventListeners, sizeof(EventListenerComponent)));
	PROPAGATE_ERROR(Bucket_Init(&level->physics, sizeof(PhysicsComponent)));
	PROPAGATE_ERROR(Bucket_Init(&level->graphics, sizeof(GraphicsComponent)));
	PROPAGATE_ERROR(Bucket_Init(&level->terrainGraphics, sizeof(GraphicsComponent)));
	PROPAGATE_ERROR(Bucket_Init(&level->defenses, sizeof(ComponentDefense)));
	PROPAGATE_ERROR(Bucket_Init(&level->offenses, sizeof(ComponentOffense)));
	level->world = Box2DWorldCreate((Vec2F) { 0.0f, 0.0f });
	level->contactListener = Box2DContactListenerRegister(PhysicsComponentContactCB);
	Box2DWorldSetContactListener(level->world, level->contactListener);
	PROPAGATE_ERROR(Array_Init(&level->deleteList, sizeof(ID), 16, UINT16_MAX + 1));
	PROPAGATE_ERROR(Bucket_Init(&level->lightSources, sizeof(ComponentLightSource)));
	PROPAGATE_ERROR(SpatialMapInit(&level->lightSourceSpatialMap, sizeof(ID)));
	PROPAGATE_ERROR(Bucket_Init(&level->prePhysicsStopwatches, sizeof(Array)));
	return 0;
}

void LevelDeleteMarkedObjects(Level* level) {
	for (size_t i = 0; i < level->deleteList.length; i++) {
		ID* objIdPtr = Array_Get(&level->deleteList, i);
		if (objIdPtr) {
			ID objId = *objIdPtr;
			Object* obj = Bucket_GetById(&level->objects, objId);
			if (obj) {
				ObjectDeinit(obj);
				Bucket_Unmark(&level->objects, obj);
			}
		}
	}
	Array_Clear(&level->deleteList);
}

void LevelDeinit(Level* level) {
	// TODO delete members in objects
	Bucket_Term(&level->prePhysicsStopwatches);
	SpatialMapDeinit(&level->lightSourceSpatialMap);
	Bucket_Term(&level->lightSources);
	PathfinderMapDeinit(&level->pathfinderMap);
	Box2DContactListenerDestroy(level->contactListener);
	Box2DWorldDestroy(level->world);
	Array_Term(&level->deleteList);
	Bucket_Term(&level->offenses);
	Bucket_Term(&level->defenses);
	Bucket_Term(&level->terrainGraphics);
	Bucket_Term(&level->graphics);
	Bucket_Term(&level->physics);
	Bucket_Term(&level->eventListeners);
	InsertionListDeinit(&level->drawList);
	Bucket_Term(&level->objects);
	memset(level, 0, sizeof(Level));
}

int LevelLoadTest(Level* level) {
	TerrainLoad("resources/terrains/test.txt");

	Object* player = Bucket_Mark(&level->objects, NULL, &level->playerId);
	ObjectPlayerInit(player); // TODO check return value

	Object* camera = Bucket_Mark(&level->objects, NULL, &level->cameraId);
	ObjectCameraInit(camera);

	const unsigned skeletonCount = 100;
	for (unsigned i = 0; i < skeletonCount; i++) {
		Object* skeleton = Bucket_Mark(&level->objects, NULL, NULL);
		ObjectEnemyInit(skeleton, (Vec2F) { (float)i, -10.0f });
	}

	/*Object* wall = Bucket_Mark(&level->objects, NULL, NULL);
	ObjectWallInit(wall, (Vec2F) { 0.0f, -2.0f });

	Object* box = Bucket_Mark(&level->objects, NULL, NULL);
	ObjectStaticBoxInit(box, (Vec2F) { -2.0f, 0.0f });*/

	return 0;
}

int LevelLoadEditor(Level* level) {
	TerrainLoad("resources/terrains/test.txt");

	Object* god = Bucket_Mark(&level->objects, NULL, &level->playerId);
	ObjectGodInit(god); // TODO check return value

	Object* camera = Bucket_Mark(&level->objects, NULL, &level->cameraId);
	ObjectCameraInit(camera);

	Object* skeleton = Bucket_Mark(&level->objects, NULL, NULL);
	ObjectEnemyInit(skeleton, (Vec2F) { -2.0f, -2.0f });

	Object* wall = Bucket_Mark(&level->objects, NULL, NULL);
	ObjectWallInit(wall, (Vec2F) { 0.0f, -2.0f });

	Object* box = Bucket_Mark(&level->objects, NULL, NULL);
	ObjectStaticBoxInit(box, (Vec2F) { -2.0f, 0.0f });

	return 0;
}
