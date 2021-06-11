#ifndef TERRAIN_H
#define TERRAIN_H

#include "GameObject.h"
#include "Array.h"
#include "Error.h"

int TerrainInit(GameObject*obj);
void TerrainSetTiles(GameObject* obj, Array tiles, size_t colCount);
int TerrainGenerateTexture(GameObject* obj);

#endif
