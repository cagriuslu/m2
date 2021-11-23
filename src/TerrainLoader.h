#ifndef TERRAIN_LOADER_H
#define TERRAIN_LOADER_H

#include "Level.h"

int TerrainLoader_LoadTiles(Game *game, Level* level, const char* tname);
int TerrainLoader_LoadEnemies(Game *game, Level* level, const char* tname);

#endif
