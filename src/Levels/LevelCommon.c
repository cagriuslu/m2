#include "LevelCommon.h"

void (*gUnloader)();

void LevelSetUnloader(void (*unloader)()) {
	gUnloader = unloader;
}

void LevelCallUnloader() {
	if (gUnloader) {
		gUnloader();
	}
}

int LevelLoadTerrainCameraPlayer() {
	Object* terrain = CreateObject(); // 0
	Object *camera = CreateObject(); // 1
	Object *player = DrawObject(CreateObject()); // 2
	TerrainInit(terrain);
	CameraInit(camera, player);
	PlayerInit(player);

	// TODO return proper error
	return 0;
}

int LevelLoadTerrainCameraGod() {
	Object *terrain = CreateObject(); // 0
	Object *camera = CreateObject(); // 1
	Object *god = CreateObject(); // 2
	TerrainInit(terrain);
	CameraInit(camera, god);
	GodInit(god);

	// TODO return proper error
	return 0;
}
