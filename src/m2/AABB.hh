#ifndef AABB_H
#define AABB_H

#include <m2/Vec2F-old.hh>

typedef struct {
	Vec2F lowerBound;
	Vec2F upperBound;
} AABB;

#endif
