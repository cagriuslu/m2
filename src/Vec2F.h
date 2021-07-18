#ifndef VEC2F_H
#define VEC2F_H

#include "Vec2I.h"

typedef struct _Vec2F {
	float x, y;
} Vec2F;

Vec2F Vec2F_Add(Vec2F lhs, Vec2F rhs);
Vec2F Vec2F_Sub(Vec2F lhs, Vec2F rhs);
Vec2F Vec2F_Mul(Vec2F lhs, float rhs);
float Vec2F_Length(Vec2F in);
Vec2F Vec2F_Normalize(Vec2F in);
float Vec2F_Distance(Vec2F lhs, Vec2F rhs);
float Vec2F_AngleRads(Vec2F vector);

bool Vec2F_Equals(Vec2F lhs, Vec2F rhs);

Vec2I Vec2F_To2I(Vec2F v);
Vec2F Vec2F_FromVec2I(Vec2I v);

#endif
