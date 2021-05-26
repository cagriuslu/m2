#ifndef LEVEL_COMMON_H
#define LEVEL_COMMON_H

#include "../Main.h"
#include "../Array.h"
#include "../DrawList.h"
#include "../Terrain.h"
#include "../Camera.h"
#include "../Player.h"
#include "../God.h"
#include "../Blueprint.h"

void LevelSetUnloader(void (*unloader)());
void LevelCallUnloader();

int LevelLoadTerrainCameraPlayer();
int LevelLoadTerrainCameraGod();

#endif
