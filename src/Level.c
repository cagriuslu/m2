#include "Level.h"
#include "Object.h"
#include "Component.h"
#include "Box2DWrapper.h"
#include "TerrainLoader.h"
#include <stdio.h>
#include <string.h>

int LevelInit(Level* level) {
	memset(level, 0, sizeof(Level));
	PROPAGATE_ERROR(BucketInit(&level->objects, sizeof(Object)));
	PROPAGATE_ERROR(InsertionListInit(&level->drawList, UINT16_MAX + 1, GraphicsComponentYComparatorCB));
	PROPAGATE_ERROR(BucketInit(&level->eventListeners, sizeof(EventListenerComponent)));
	PROPAGATE_ERROR(BucketInit(&level->physics, sizeof(PhysicsComponent)));
	PROPAGATE_ERROR(BucketInit(&level->graphics, sizeof(GraphicsComponent)));
	PROPAGATE_ERROR(BucketInit(&level->terrainGraphics, sizeof(GraphicsComponent)));
	PROPAGATE_ERROR(BucketInit(&level->defenses, sizeof(ComponentDefense)));
	PROPAGATE_ERROR(BucketInit(&level->offenses, sizeof(ComponentOffense)));
	PROPAGATE_ERROR(ArrayInit(&level->deleteList, sizeof(uint32_t), 16, UINT16_MAX + 1));
	level->world = Box2DWorldCreate((Vec2F) { 0.0f, 0.0f });
	level->contactListener = Box2DContactListenerRegister(PhysicsComponentContactCB);
	Box2DWorldSetContactListener(level->world, level->contactListener);
	fprintf(stderr, "Level initialized\n");
	return 0;
}

void LevelDeleteObjects(Level* level) {
	for (size_t i = 0; i < level->deleteList.length; i++) {
		uint32_t* objIdPtr = ArrayGet(&level->deleteList, i);
		if (objIdPtr) {
			uint32_t objId = *objIdPtr;
			Object* obj = BucketGetById(&level->objects, objId);
			if (obj) {
				ObjectDeinit(obj);
				BucketUnmark(&level->objects, obj);
			}
		}
	}
	ArrayClear(&level->deleteList);
}

void LevelDeinit(Level* level) {
	// TODO delete members in objects
	Box2DContactListenerDestroy(level->contactListener);
	Box2DWorldDestroy(level->world);
	ArrayDeinit(&level->deleteList);
	BucketDeinit(&level->offenses);
	BucketDeinit(&level->defenses);
	BucketDeinit(&level->terrainGraphics);
	BucketDeinit(&level->graphics);
	BucketDeinit(&level->physics);
	BucketDeinit(&level->eventListeners);
	InsertionListDeinit(&level->drawList);
	BucketDeinit(&level->objects);
	memset(level, 0, sizeof(Level));
	fprintf(stderr, "Level deinitialized\n");
}

int LevelLoadTest(Level* level) {
	TerrainLoad("resources/terrains/test.txt");

	Object* player = BucketMark(&level->objects, NULL, &level->playerId);
	ObjectPlayerInit(player); // TODO check return value

	Object* camera = BucketMark(&level->objects, NULL, &level->cameraId);
	ObjectCameraInit(camera);

	Object* skeleton = BucketMark(&level->objects, NULL, NULL);
	ObjectSkeletonInit(skeleton, (Vec2F) { -2.0f, -2.0f });

	Object* wall = BucketMark(&level->objects, NULL, NULL);
	ObjectWallInit(wall, (Vec2F) { 0.0f, -2.0f });

	Object* box = BucketMark(&level->objects, NULL, NULL);
	ObjectStaticBoxInit(box, (Vec2F) { -2.0f, 0.0f });

	return 0;
}

int LevelLoadEditor(Level* level) {
	TerrainLoad("resources/terrains/test.txt");

	Object* god = BucketMark(&level->objects, NULL, &level->playerId);
	ObjectGodInit(god); // TODO check return value

	Object* camera = BucketMark(&level->objects, NULL, &level->cameraId);
	ObjectCameraInit(camera);

	Object* skeleton = BucketMark(&level->objects, NULL, NULL);
	ObjectSkeletonInit(skeleton, (Vec2F) { -2.0f, -2.0f });

	Object* wall = BucketMark(&level->objects, NULL, NULL);
	ObjectWallInit(wall, (Vec2F) { 0.0f, -2.0f });

	Object* box = BucketMark(&level->objects, NULL, NULL);
	ObjectStaticBoxInit(box, (Vec2F) { -2.0f, 0.0f });

	return 0;
}
