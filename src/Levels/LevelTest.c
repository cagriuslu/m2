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
	BlueprintStaticBoxInit(staticBox1, (Vec2F) {5.0, 5.0});
	DrawListInsert(drawList, staticBox1);

	LevelSetUnloader(LevelTestUnloader);
	return 0;
}
