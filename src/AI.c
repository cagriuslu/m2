#include "AI.h"
#include "Vec2I.h"
#include "Error.h"
#include <string.h>

int AIInit(AI* ai) {
	memset(ai, 0, sizeof(AI));
	PROPAGATE_ERROR(ListInit(&ai->reversedVec2IWaypointList, sizeof(Vec2I)));
	return 0;
}

void AIDeinit(AI* ai) {
	ListDeinit(&ai->reversedVec2IWaypointList);
	memset(ai, 0, sizeof(AI));
}
