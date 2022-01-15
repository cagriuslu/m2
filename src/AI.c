#include "AI.h"
#include "Vec2I.h"
#include <string.h>

XErr AI_Init(AI* ai) {
	memset(ai, 0, sizeof(AI));
	XERR_REFLECT(List_Init(&ai->reversedWaypointList, sizeof(Vec2I)));
	return 0;
}

void AI_Term(AI* ai) {
	List_Term(&ai->reversedWaypointList);
	memset(ai, 0, sizeof(AI));
}
