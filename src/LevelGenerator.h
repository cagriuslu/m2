#ifndef LEVEL_GENERATOR_H
#define LEVEL_GENERATOR_H

#include "Level.h"

typedef enum _LevelGeneratorMapType {
	MAP_TYPE_NEIGHBORING_RECTANGLES,
} LevelGeneratorMapType;

typedef struct _LevelGeneratorParameters {
	LevelGeneratorMapType mapType;
	unsigned depth;
	// MAP_TYPE_NEIGHBORING_RECTANGLES
	float rectSideMin;
	float rectSideMax;
	float rectToRectDistanceMin;
	float rectToRectDistanceMax;
} LevelGeneratorParameters;

int LevelGeneratorGenerate(Level* level, LevelGeneratorParameters params);

#endif
