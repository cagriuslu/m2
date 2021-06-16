#include "Level.h"
#include "Object.h"
#include "EventListenerComponent.h"
#include "PhysicsComponent.h"
#include "GraphicsComponent.h"
#include "Box2DWrapper.h"
#include "TerrainLoader.h"
#include <stdio.h>
#include <string.h>

int LevelInit(Level* level) {
	memset(level, 0, sizeof(Level));
	PROPAGATE_ERROR(BucketInit(&level->objects, sizeof(Object)));
	PROPAGATE_ERROR(InsertionListInit(&level->drawList, sizeof(uint32_t), UINT16_MAX + 1, GraphicsComponentYComparatorCB));
	PROPAGATE_ERROR(BucketInit(&level->eventListeners, sizeof(EventListenerComponent)));
	PROPAGATE_ERROR(BucketInit(&level->physics, sizeof(PhysicsComponent)));
	PROPAGATE_ERROR(BucketInit(&level->graphics, sizeof(GraphicsComponent)));
	PROPAGATE_ERROR(BucketInit(&level->terrainGraphics, sizeof(GraphicsComponent)));
	level->world = Box2DWorldCreate((Vec2F) { 0.0f, 0.0f });
	level->contactListener = Box2DContactListenerRegister(PhysicsComponentContactCB);
	Box2DWorldSetContactListener(level->world, level->contactListener);
	fprintf(stderr, "Level initialized\n");
	return 0;
}

void LevelDeinit(Level* level) {
	// TODO delete members in objects
	Box2DContactListenerDestroy(level->contactListener);
	Box2DWorldDestroy(level->world);
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
