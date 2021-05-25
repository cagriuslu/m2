#include "../Level.h"

void LevelEditorUnloader() {
	// TODO
}

int LevelEditorLoad() {
	PROPAGATE_ERROR(LevelLoadTerrainCameraGod());

	Array *objArray = CurrentObjectArray();
	DrawList *drawList = CurrentDrawList();
	
	// Test object
	Object *staticBox1 = ArrayAppend(objArray, NULL);
	StaticBoxInit(staticBox1, (Vec2F) {5.0, 5.0});
	DrawListInsert(drawList, staticBox1);

	LevelSetUnloader(LevelEditorUnloader);
	return 0;
}
