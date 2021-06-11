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
	GameObject* terrain = CreateObject(); // 0
	GameObject *camera = CreateObject(); // 1
	GameObject *player = DrawObject(CreateObject()); // 2
	TerrainInit(terrain);
	CameraInit(camera, player);
	PlayerInit(player);

	// TODO return proper error
	return 0;
}

int LevelLoadTerrainCameraGod() {
	GameObject *terrain = CreateObject(); // 0
	GameObject *camera = CreateObject(); // 1
	GameObject *god = CreateObject(); // 2
	TerrainInit(terrain);
	CameraInit(camera, god);
	GodInit(god);

	// TODO return proper error
	return 0;
}
