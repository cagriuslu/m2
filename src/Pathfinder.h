#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "Level.h"

typedef struct _PathfinderMap {
	int dummy;
} PathfinderMap;

int PathfinderMapFromLevel(PathfinderMap* pm, Level* level);

#endif
