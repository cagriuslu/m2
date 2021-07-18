#include "Vec2F.h"
#include <math.h>

Vec2F Vec2F_Add(Vec2F lhs, Vec2F rhs) {
	return (Vec2F) {lhs.x + rhs.x, lhs.y + rhs.y};
}

Vec2F Vec2F_Sub(Vec2F lhs, Vec2F rhs) {
	return (Vec2F) {lhs.x - rhs.x, lhs.y - rhs.y};
}

Vec2F Vec2F_Mul(Vec2F lhs, float rhs) {
	return (Vec2F) {lhs.x * rhs, lhs.y * rhs};
}

float Vec2F_Length(Vec2F in) {
	if (in.x == 0.0f && in.y == 0.0f) {
		return 0.0f;
	}
	return sqrtf(powf(in.x, 2.0f) + powf(in.y, 2.0f));
}

Vec2F Vec2F_Normalize(Vec2F in) {
	if (in.x == 0.0f && in.y == 0.0f) {
		return in;
	}
	float len = Vec2F_Length(in);
	return (Vec2F) { in.x / len, in.y / len };
}

float Vec2F_Distance(Vec2F lhs, Vec2F rhs) {
	return Vec2F_Length(Vec2F_Sub(lhs, rhs));
}

float Vec2F_AngleRads(Vec2F vector) {
	return atan2f(vector.y, vector.x);
}

bool Vec2F_Equals(Vec2F lhs, Vec2F rhs) {
	return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

Vec2I Vec2F_To2I(Vec2F v) {
	return (Vec2I) {(int32_t) roundf(v.x), (int32_t) roundf(v.y)};
}

Vec2F Vec2F_FromVec2I(Vec2I v) {
	return (Vec2F) { (float)v.x, (float)v.y };
}
