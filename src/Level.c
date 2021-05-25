#include "Level.h"

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

	// TODO
	return 0;
}
