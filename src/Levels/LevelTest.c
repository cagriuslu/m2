#include "../Level.h"
#include "LevelCommon.h"

void LevelTestUnloader() {
	// TODO
}

int LevelTestLoad() {
	PROPAGATE_ERROR(LevelLoadTerrainCameraPlayer());

	Array *objArray = CurrentObjectArray();
	DrawList *drawList = CurrentDrawList();

	// Test object
	Object *staticBox1 = ArrayAppend(objArray, NULL);
	StaticBoxInit(staticBox1, (Vec2F) {5.0, 5.0});
	DrawListInsert(drawList, staticBox1);

	Object *wall1 = ArrayAppend(objArray, NULL);
	WallInit(wall1, (Vec2F) {2.0, 2.0});
	DrawListInsert(drawList, wall1);
	Object *wall2 = ArrayAppend(objArray, NULL);
	WallInit(wall2, (Vec2F) {3.0, 2.0});
	DrawListInsert(drawList, wall2);
	Object *wall3 = ArrayAppend(objArray, NULL);
	WallInit(wall3, (Vec2F) {3.0, 3.0});
	DrawListInsert(drawList, wall3);

	LevelSetUnloader(LevelTestUnloader);
	return 0;
}
