#ifndef TERRAIN_LOADER_H
#define TERRAIN_LOADER_H

#include "Level.h"

int TerrainLoader_LoadTiles(Level* level, const char* tname);
int TerrainLoader_LoadEnemies(Level* level, const char* tname);

#endif
