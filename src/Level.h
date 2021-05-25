#ifndef LEVEL_H
#define LEVEL_H

#include "Main.h"
#include "Array.h"
#include "DrawList.h"
#include "Terrain.h"
#include "Camera.h"
#include "Player.h"
#include "Blueprint.h"

#define TERRAIN_INDEX (0)
#define CAMERA_INDEX (1)
#define PLAYER_INDEX (2)

int LevelTestLoad();
int LevelEditorLoad();

int LevelLoadTerrainCameraPlayer();

#endif
