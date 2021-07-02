#ifndef VEC2F_H
#define VEC2F_H

#include "Vec2I.h"

typedef struct _Vec2F {
	float x, y;
} Vec2F;

Vec2F Vec2FAdd(Vec2F lhs, Vec2F rhs);
Vec2F Vec2FSub(Vec2F lhs, Vec2F rhs);
Vec2F Vec2FMul(Vec2F lhs, float rhs);
float Vec2FLength(Vec2F in);
Vec2F Vec2FNormalize(Vec2F in);
float Vec2FDistance(Vec2F lhs, Vec2F rhs);
float Vec2FAngleRads(Vec2F vector);

bool Vec2FEquals(Vec2F lhs, Vec2F rhs);

Vec2I Vec2FTo2I(Vec2F v);
Vec2F Vec2FFromVec2I(Vec2I v);

#endif
