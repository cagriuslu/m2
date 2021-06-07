#include "../Level.h"
#include "LevelCommon.h"
#include "../TerrainLoader.h"

void LevelTestUnloader() {
	// TODO
}

int LevelTestLoad() {
	PROPAGATE_ERROR(LevelLoadTerrainCameraPlayer());

	Object* terrain = ObjectStoreGetObjectByIndex(CurrentObjectStore(), TERRAIN_INDEX);
	LoadTerrain(terrain, "resources/terrains/test.txt");

	// Test object
	Object *staticBox1 = DrawObject(CreateObject());
	BlueprintStaticBoxInit(staticBox1, (Vec2F) {6.0, 6.0});

	// Test object
	Object* wall1 = DrawObject(CreateObject());
	BlueprintWallInit(wall1, (Vec2F) { 8.0, 6.0 });
	Object* wall2 = DrawObject(CreateObject());
	BlueprintWallInit(wall2, (Vec2F) { 9.0, 6.0 });
	Object* wall3 = DrawObject(CreateObject());
	BlueprintWallInit(wall3, (Vec2F) { 10.0, 6.0 });

	// Test enemy
	Object* skeleton1 = DrawObject(CreateObject());
	BlueprintSkeletonInit(skeleton1, (Vec2F) {-2, -2});

	// Test bullet
	Object* bullet1 = DrawObject(CreateObject());
	BlueprintBulletInit(bullet1, (Vec2F) {-4, -4}, (Vec2F) {1, 1});

	LevelSetUnloader(LevelTestUnloader);
	return 0;
}
