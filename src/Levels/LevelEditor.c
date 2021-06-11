#include "../Level.h"
#include "LevelCommon.h"

void LevelEditorUnloader() {
	// TODO
}

int LevelEditorLoad() {
	PROPAGATE_ERROR(LevelLoadTerrainCameraGod());

	// Test object
	GameObject *staticBox1 = DrawObject(CreateObject());
	BlueprintStaticBoxInit(staticBox1, (Vec2F) {5.0, 5.0});

	LevelSetUnloader(LevelEditorUnloader);
	return 0;
}
