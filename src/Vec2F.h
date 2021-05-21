#ifndef VEC2F_H
#define VEC2F_H

#include "Vec2I.h"

typedef struct _Vec2F {
	float x, y;
} Vec2F;

Vec2F Vec2FAdd(Vec2F lhs, Vec2F rhs);
Vec2F Vec2FSub(Vec2F lhs, Vec2F rhs);
Vec2F Vec2FMul(Vec2F lhs, float rhs);

Vec2I Vec2Fto2I(Vec2F v);

#endif
