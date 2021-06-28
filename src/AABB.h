#ifndef AABB_H
#define AABB_H

#include "Vec2F.h"

typedef struct _AABB {
	Vec2F lowerBound;
	Vec2F upperBound;
} AABB;

#endif
