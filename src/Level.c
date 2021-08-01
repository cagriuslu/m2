#include "Level.h"
#include "Object.h"
#include "Component.h"
#include "Box2D.h"
#include "TerrainLoader.h"
#include <stdio.h>
#include <string.h>

int Level_Init(Level* level) {
	memset(level, 0, sizeof(Level));
	PROPAGATE_ERROR(Bucket_Init(&level->objects, sizeof(Object)));
	PROPAGATE_ERROR(InsertionList_Init(&level->drawList, UINT16_MAX + 1, GraphicsComponent_YComparatorCB));
	PROPAGATE_ERROR(Bucket_Init(&level->eventListeners, sizeof(ComponentEventListener)));
	PROPAGATE_ERROR(Bucket_Init(&level->physics, sizeof(ComponentPhysics)));
	PROPAGATE_ERROR(Bucket_Init(&level->graphics, sizeof(ComponentGraphics)));
	PROPAGATE_ERROR(Bucket_Init(&level->terrainGraphics, sizeof(ComponentGraphics)));
	PROPAGATE_ERROR(Bucket_Init(&level->defenses, sizeof(ComponentDefense)));
	PROPAGATE_ERROR(Bucket_Init(&level->offenses, sizeof(ComponentOffense)));
	level->world = Box2DWorldCreate((Vec2F) { 0.0f, 0.0f });
	level->contactListener = Box2DContactListenerRegister(PhysicsComponent_ContactCB);
	Box2DWorldSetContactListener(level->world, level->contactListener);
	PROPAGATE_ERROR(Array_Init(&level->deleteList, sizeof(ID), 16, UINT16_MAX + 1));
	PROPAGATE_ERROR(Bucket_Init(&level->lightSources, sizeof(ComponentLightSource)));
	PROPAGATE_ERROR(SpatialMap_Init(&level->lightSourceSpatialMap, sizeof(ID)));
	return 0;
}

void Level_DeleteMarkedObjects(Level* level) {
	for (size_t i = 0; i < level->deleteList.length; i++) {
		ID* objIdPtr = Array_Get(&level->deleteList, i);
		if (objIdPtr) {
			ID objId = *objIdPtr;
			Object* obj = Bucket_GetById(&level->objects, objId);
			if (obj) {
				Object_Term(obj);
				Bucket_Unmark(&level->objects, obj);
			}
		}
	}
	Array_Clear(&level->deleteList);
}

void Level_Term(Level* level) {
	// TODO delete members in objects
	SpatialMap_Term(&level->lightSourceSpatialMap);
	Bucket_Term(&level->lightSources);
	PathfinderMap_Term(&level->pathfinderMap);
	Box2DContactListenerDestroy(level->contactListener);
	Box2DWorldDestroy(level->world);
	Array_Term(&level->deleteList);
	Bucket_Term(&level->offenses);
	Bucket_Term(&level->defenses);
	Bucket_Term(&level->terrainGraphics);
	Bucket_Term(&level->graphics);
	Bucket_Term(&level->physics);
	Bucket_Term(&level->eventListeners);
	InsertionList_Term(&level->drawList);
	Bucket_Term(&level->objects);
	memset(level, 0, sizeof(Level));
}

int Level_LoadTest(Level* level) {
	TerrainLoader_LoadTiles(level, "resources/terrains/test.txt");

	Object* player = Bucket_Mark(&level->objects, NULL, &level->playerId);
	ObjectPlayer_Init(player);

	Object* camera = Bucket_Mark(&level->objects, NULL, &level->cameraId);
	ObjectCamera_Init(camera);

	const unsigned skeletonCount = 100;
	for (unsigned i = 0; i < skeletonCount; i++) {
		Object* skeleton = Bucket_Mark(&level->objects, NULL, NULL);
		ObjectEnemy_Init(skeleton, (Vec2F) { (float)i, -10.0f }, NULL);
	}

	/*Object* wall = Bucket_Mark(&level->objects, NULL, NULL);
	ObjectWall_Init(wall, (Vec2F) { 0.0f, -2.0f });

	Object* box = Bucket_Mark(&level->objects, NULL, NULL);
	ObjectStaticBox_Init(box, (Vec2F) { -2.0f, 0.0f });*/

	TerrainLoader_LoadEnemies(level, "resources/terrains/test.txt");

	return 0;
}

int Level_LoadEditor(Level* level) {
	TerrainLoader_LoadTiles(level, "resources/terrains/test.txt");

	Object* god = Bucket_Mark(&level->objects, NULL, &level->playerId);
	ObjectGod_Init(god); // TODO check return value

	Object* camera = Bucket_Mark(&level->objects, NULL, &level->cameraId);
	ObjectCamera_Init(camera);

	Object* skeleton = Bucket_Mark(&level->objects, NULL, NULL);
	ObjectEnemy_Init(skeleton, (Vec2F) { -2.0f, -2.0f }, NULL);

	Object* wall = Bucket_Mark(&level->objects, NULL, NULL);
	ObjectWall_Init(wall, (Vec2F) { 0.0f, -2.0f });

	Object* box = Bucket_Mark(&level->objects, NULL, NULL);
	ObjectStaticBox_Init(box, (Vec2F) { -2.0f, 0.0f });

	return 0;
}
