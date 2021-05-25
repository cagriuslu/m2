#include "../Level.h"

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

	LevelSetUnloader(LevelTestUnloader);
	return 0;
}
