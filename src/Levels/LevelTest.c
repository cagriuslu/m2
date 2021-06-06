#include "../Level.h"
#include "LevelCommon.h"
#include "../TerrainLoader.h"

void LevelTestUnloader() {
	// TODO
}

int LevelTestLoad() {
	PROPAGATE_ERROR(LevelLoadTerrainCameraPlayer());

	Array* objArray = CurrentObjectArray();
	DrawList* drawList = CurrentDrawList();

	Object* terrain = ArrayGet(objArray, 0);
	LoadTerrain(terrain, "resources/terrains/test.txt");

	// Test object
	Object *staticBox1 = ArrayAppend(objArray, NULL);
	BlueprintStaticBoxInit(staticBox1, (Vec2F) {6.0, 6.0});
	DrawListInsert(drawList, staticBox1);

	// Test object
	Object* wall1 = ArrayAppend(objArray, NULL);
	BlueprintWallInit(wall1, (Vec2F) { 8.0, 6.0 });
	DrawListInsert(drawList, wall1);
	Object* wall2 = ArrayAppend(objArray, NULL);
	BlueprintWallInit(wall2, (Vec2F) { 9.0, 6.0 });
	DrawListInsert(drawList, wall2);
	Object* wall3 = ArrayAppend(objArray, NULL);
	BlueprintWallInit(wall3, (Vec2F) { 10.0, 6.0 });
	DrawListInsert(drawList, wall3);

	// Test enemy
	Object* skeleton1 = ArrayAppend(objArray, NULL);
	BlueprintSkeletonInit(skeleton1, (Vec2F) {-2, -2});
	DrawListInsert(drawList, skeleton1);

	// Test bullet
	Object* bullet1 = ArrayAppend(objArray, NULL);
	BlueprintBulletInit(bullet1, (Vec2F) {-4, -4}, (Vec2F) {1, 1});
	DrawListInsert(drawList, bullet1);

	LevelSetUnloader(LevelTestUnloader);
	return 0;
}
