#ifndef VEC2F_H
#define VEC2F_H

#include "Vec2I.h"
#include <math.h>

typedef struct _Vec2F {
	float x, y;
} Vec2F;

#define VEC2F_ZERO ((Vec2F){0.0f, 0.0f})
#define VEC2F_NAN ((Vec2F){NAN, NAN})
#define VEC2F(x,y) ((Vec2F){(float)(x), (float)(y)})

Vec2F Vec2F_Add(Vec2F lhs, Vec2F rhs);
Vec2F Vec2F_Sub(Vec2F lhs, Vec2F rhs);
Vec2F Vec2F_Mul(Vec2F lhs, float rhs);
Vec2F Vec2F_Div(Vec2F lhs, float rhs);
float Vec2F_Length(Vec2F in);
Vec2F Vec2F_SetLength(Vec2F in, float len);
Vec2F Vec2F_FloorLength(Vec2F in, float len);
Vec2F Vec2F_CeilLength(Vec2F in, float len);
Vec2F Vec2F_Normalize(Vec2F in);
float Vec2F_Distance(Vec2F lhs, Vec2F rhs);
Vec2F Vec2F_Lerp(Vec2F from, Vec2F to, float ratio);
float Vec2F_AngleRads(Vec2F vector);
Vec2F Vec2F_FromAngle(float rads);

bool Vec2F_Equals(Vec2F lhs, Vec2F rhs);
bool Vec2F_IsNan(Vec2F in);

Vec2F Vec2F_FromVec2I(Vec2I v);

#endif
