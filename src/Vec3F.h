#ifndef VEC3F_H
#define VEC3F_H

#include "Vec2I.h"

typedef struct _Vec3F {
	float x, y, z;
} Vec3F;

Vec3F Vec3FAdd(Vec3F lhs, Vec3F rhs);
Vec3F Vec3FSub(Vec3F lhs, Vec3F rhs);
Vec3F Vec3FMul(Vec3F lhs, float rhs);

Vec2I Vec3Fto2I(Vec3F v);

#endif
