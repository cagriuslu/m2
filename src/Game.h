#ifndef GAME_H
#define GAME_H

#include "Window.h"

typedef struct _Game {
	int tileWidth;
	const char* textureImageFilePath;
	const char* textureMetaImageFilePath;
	const char* textureMetaFilePath;
	Window window;
	float physicsStepPerSecond;
	float physicsStepPeriod;
	int velocityIterations;
	int positionIterations;
} Game;

#endif
