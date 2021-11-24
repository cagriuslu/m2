#include "Level.h"
#include "Object.h"
#include "Component.h"
#include "Box2D.h"
#include "TerrainLoader.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int Level_Init(Level* level) {
	memset(level, 0, sizeof(Level));
	PROPAGATE_ERROR(Pool_Init(&level->objects, 16, sizeof(Object)));
	PROPAGATE_ERROR(InsertionList_Init(&level->drawList, UINT16_MAX + 1, GraphicsComponent_YComparatorCB));
	PROPAGATE_ERROR(Pool_Init(&level->eventListeners, 16, sizeof(ComponentEventListener)));
	PROPAGATE_ERROR(Pool_Init(&level->physics, 16, sizeof(ComponentPhysics)));
	PROPAGATE_ERROR(Pool_Init(&level->graphics, 16, sizeof(ComponentGraphics)));
	PROPAGATE_ERROR(Pool_Init(&level->terrainGraphics, 16, sizeof(ComponentGraphics)));
	PROPAGATE_ERROR(Pool_Init(&level->defenses, 16, sizeof(ComponentDefense)));
	PROPAGATE_ERROR(Pool_Init(&level->offenses, 16, sizeof(ComponentOffense)));
	level->world = Box2DWorldCreate((Vec2F) { 0.0f, 0.0f });
	level->contactListener = Box2DContactListenerRegister(PhysicsComponent_ContactCB);
	Box2DWorldSetContactListener(level->world, level->contactListener);
	PROPAGATE_ERROR(Array_Init(&level->deleteList, sizeof(ID), 16, UINT16_MAX + 1, NULL));
	PROPAGATE_ERROR(Pool_Init(&level->lightSources, 16, sizeof(ComponentLightSource)));
	PROPAGATE_ERROR(SpatialMap_Init(&level->lightSourceSpatialMap, sizeof(ID)));
	return 0;
}

void Level_DeleteMarkedObjects(Level* level) {
	for (size_t i = 0; i < level->deleteList.length; i++) {
		ID* objIdPtr = Array_Get(&level->deleteList, i);
		if (objIdPtr) {
			ID objId = *objIdPtr;
			Object* obj = Pool_GetById(&level->objects, objId);
			if (obj) {
				Object_Term(obj);
				Pool_Unmark(&level->objects, obj);
			}
		}
	}
	Array_Clear(&level->deleteList);
}

void Level_Term(Level* level) {
	// TODO delete members in objects
	SpatialMap_Term(&level->lightSourceSpatialMap);
	Pool_Term(&level->lightSources);
	PathfinderMap_Term(&level->pathfinderMap);
	Box2DContactListenerDestroy(level->contactListener);
	Box2DWorldDestroy(level->world);
	Array_Term(&level->deleteList);
	Pool_Term(&level->offenses);
	Pool_Term(&level->defenses);
	Pool_Term(&level->terrainGraphics);
	Pool_Term(&level->graphics);
	Pool_Term(&level->physics);
	Pool_Term(&level->eventListeners);
	InsertionList_Term(&level->drawList);
	Pool_Term(&level->objects);
	memset(level, 0, sizeof(Level));
}

int Level_LoadTest(Level* level) {
	level->levelType = LEVEL_TYPE_SINGLE_PLAYER;
	
	TerrainLoader_LoadTiles(level, "resources/terrains/test.txt");

	Array standardItemSet;
	Array_Init(&standardItemSet, sizeof(Item), 16, UINT32_MAX, NULL);
	Item_GenerateStandardItemSet(&standardItemSet);
	Character* character = malloc(sizeof(Character));
	assert(character);
	Character_Init(character, CHARTYP_HUMAN, 1, standardItemSet);
	Character_Preprocess(character);

	Object* player = Pool_Mark(&level->objects, NULL, &level->playerId);
	ObjectPlayer_Init(player, character);

	Hud_Init(&level->hud, level);

	Object* camera = Pool_Mark(&level->objects, NULL, &level->cameraId);
	ObjectCamera_Init(camera);

	const unsigned skeletonCount = 100;
	for (unsigned i = 0; i < skeletonCount; i++) {
		Object* skeleton = Pool_Mark(&level->objects, NULL, NULL);
		ObjectEnemy_Init(skeleton, (Vec2F) { (float)i, -10.0f }, NULL);
	}

	/*Object* wall = Pool_Mark(&level->objects, NULL, NULL);
	ObjectWall_Init(wall, (Vec2F) { 0.0f, -2.0f });

	Object* box = Pool_Mark(&level->objects, NULL, NULL);
	ObjectStaticBox_Init(box, (Vec2F) { -2.0f, 0.0f });*/

	TerrainLoader_LoadEnemies(level, "resources/terrains/test.txt");

	return 0;
}

int Level_LoadEditor(Level* level) {
	level->levelType = LEVEL_TYPE_LEVEL_EDITOR;
	
	TerrainLoader_LoadTiles(level, "resources/terrains/test.txt");

	Object* god = Pool_Mark(&level->objects, NULL, &level->playerId);
	ObjectGod_Init(god); // TODO check return value

	Object* camera = Pool_Mark(&level->objects, NULL, &level->cameraId);
	ObjectCamera_Init(camera);

	Object* skeleton = Pool_Mark(&level->objects, NULL, NULL);
	ObjectEnemy_Init(skeleton, (Vec2F) { -2.0f, -2.0f }, NULL);

	Object* wall = Pool_Mark(&level->objects, NULL, NULL);
	ObjectWall_Init(wall, (Vec2F) { 0.0f, -2.0f });

	Object* box = Pool_Mark(&level->objects, NULL, NULL);
	ObjectStaticBox_Init(box, (Vec2F) { -2.0f, 0.0f });

	return 0;
}
