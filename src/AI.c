#include "AI.h"
#include "Vec2I.h"
#include <string.h>

XErr AI_Init(AI* ai) {
	memset(ai, 0, sizeof(AI));
	PROPAGATE_ERROR(ListInit(&ai->reversedWaypointList, sizeof(Vec2I)));
	return 0;
}

void AI_Term(AI* ai) {
	ListDeinit(&ai->reversedWaypointList);
	memset(ai, 0, sizeof(AI));
}
