#include "Level.h"

void (*gUnloader)();

void LevelUnload() {
	if (gUnloader) {
		gUnloader();
	}
}

void LevelSetUnloader(void (*unloader)()) {
	gUnloader = unloader;
}

int LevelLoadTerrainCameraPlayer() {
	Array *objArray = CurrentObjectArray();
	DrawList *drawList = CurrentDrawList();

	// 0: Terrain
	// 1: Camera
	// 2: Player
	Object *terrain = ArrayAppend(objArray, NULL); 
	Object *camera = ArrayAppend(objArray, NULL);
	Object *player = ArrayAppend(objArray, NULL);
	// 0: Terrain
	TerrainInit(terrain);
	// 2: Player, Camera uses Player
	PlayerInit(player);
	DrawListInsert(drawList, player);
	// 1: Camera
	CameraInit(camera, player);

	// TODO return proper error
	return 0;
}

int LevelLoadTerrainCameraGod() {
	Array *objArray = CurrentObjectArray();
	DrawList *drawList = CurrentDrawList();

	// 0: Terrain
	// 1: Camera
	// 2: God
	Object *terrain = ArrayAppend(objArray, NULL); 
	Object *camera = ArrayAppend(objArray, NULL);
	Object *god = ArrayAppend(objArray, NULL);
	// 0: Terrain
	TerrainInit(terrain);
	// 2: Player, Camera uses God
	GodInit(god);
	// 1: Camera
	CameraInit(camera, god);

	// TODO return proper error
	return 0;
}
