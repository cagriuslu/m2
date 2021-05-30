#ifndef TERRAIN_H
#define TERRAIN_H

#include "Object.h"
#include "Array.h"
#include "Error.h"

int TerrainInit(Object *obj);
void TerrainSetTiles(Object* obj, Array tiles, size_t colCount);
int TerrainGenerateTexture(Object* obj);

#endif
