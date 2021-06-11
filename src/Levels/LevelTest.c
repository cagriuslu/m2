#include "../Level.h"
#include "LevelCommon.h"
#include "../TerrainLoader.h"

void LevelTestUnloader() {
	// TODO
}

int LevelTestLoad() {
	PROPAGATE_ERROR(LevelLoadTerrainCameraPlayer());

	GameObject* terrain = ObjectStoreGetObjectByIndex(CurrentObjectStore(), TERRAIN_INDEX);
	LoadTerrain(terrain, "resources/terrains/test.txt");

	// Test object
	//BlueprintStaticBoxInit(DrawObject(CreateObject()), (Vec2F) {6.0, 6.0});

	// Test object
	//BlueprintWallInit(DrawObject(CreateObject()), (Vec2F) { 8.0, 6.0 });
	//BlueprintWallInit(DrawObject(CreateObject()), (Vec2F) { 9.0, 6.0 });
	//BlueprintWallInit(DrawObject(CreateObject()), (Vec2F) { 10.0, 6.0 });

	// Test enemy
	BlueprintSkeletonInit(DrawObject(CreateObject()), (Vec2F) {-2, -2});

	// Test bullet
	//BlueprintBulletInit(DrawObject(CreateObject()), (Vec2F) {-4, -4}, (Vec2F) {1, 1});

	LevelSetUnloader(LevelTestUnloader);
	return 0;
}
