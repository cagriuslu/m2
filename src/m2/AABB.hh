#ifndef AABB_H
#define AABB_H

#include <m2/vec2f.hh>

typedef struct {
	m2::vec2f lowerBound;
	m2::vec2f upperBound;
} AABB;

#endif
