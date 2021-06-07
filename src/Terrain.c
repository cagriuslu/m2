#include "Terrain.h"
#include "Main.h"
#include "Array.h"
#include "Tile.h"
#include <SDL.h>
#include <assert.h>

#define AsTerrainData(terrainData) ((TerrainData*) (terrainData))

typedef struct _TerrainData {
	Array tiles;
	size_t colCount;
} TerrainData;

int TerrainInit(Object *obj) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->tx = NULL;
	obj->privData = malloc(sizeof(TerrainData));
	assert(obj->privData);
	ArrayInit(&AsTerrainData(obj->privData)->tiles, sizeof(Tile), 16, SIZE_MAX);
	AsTerrainData(obj->privData)->colCount = 0;
	return 0;
}

void TerrainSetTiles(Object* obj, Array tiles, size_t colCount) {
	AsTerrainData(obj->privData)->tiles = tiles;
	AsTerrainData(obj->privData)->colCount = colCount;
}

int TerrainGenerateTexture(Object* obj) {
	TerrainData* terrainData = obj->privData;
	size_t rowCount = terrainData->tiles.length / terrainData->colCount;
	if (obj->tx) {
		SDL_DestroyTexture(obj->tx);
	}
	obj->tx = SDL_CreateTexture(
		CurrentRenderer(),
		CurrentWindowPixelFormat(),
		SDL_TEXTUREACCESS_TARGET,
		(int) (terrainData->colCount * CurrentTileWidth()),
		(int) (rowCount * CurrentTileWidth())
	);
	assert(obj->tx);

	SDL_SetRenderTarget(CurrentRenderer(), obj->tx);
	for (size_t rowIndex = 0; rowIndex < rowCount; rowIndex++) {
		for (size_t colIndex = 0; colIndex < terrainData->colCount; colIndex++) {
			Tile* tile = ArrayGet(&terrainData->tiles, rowIndex * terrainData->colCount + colIndex);
			SDL_Rect dstrect = (SDL_Rect){
				(int) colIndex * CurrentTileWidth(),
				(int) rowIndex * CurrentTileWidth(),
				CurrentTileWidth(),
				CurrentTileWidth()
			};
			SDL_RenderCopy(CurrentRenderer(), CurrentTextureLUT(), &tile->txSrc, &dstrect);
		}
	}
	SDL_SetRenderTarget(CurrentRenderer(), NULL);

	// Texture source is the whole texture
	obj->txSrc = (SDL_Rect) {0, 0, (int) terrainData->colCount * CurrentTileWidth(), (int) rowCount * CurrentTileWidth()};
	obj->txOffset.x = (float) ((terrainData->colCount - 1) * CurrentTileWidth() / 2);
	obj->txOffset.y = (float) ((rowCount - 1) * CurrentTileWidth() / 2);
	return 0;
}
