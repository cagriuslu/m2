#ifndef AABB_H
#define AABB_H

#include <m2/vec2f.hh>

struct AABB {
	m2::vec2f lowerBound;
	m2::vec2f upperBound;
};

#endif
