#include "Terrain.h"
#include "Main.h"
#include <SDL.h>

#define AsTerrainData(terrainData) ((TerrainData*) (terrainData))

typedef struct _TerrainData {
	int dummy;
} TerrainData;

int TerrainInit(Object *obj) {
	PROPAGATE_ERROR(ObjectInit(obj));
	return 0;
}

void TerrainDeinit(Object *obj) {
	ObjectDeinit(obj);
}
