#include "Vec2F.h"
#include <math.h>

Vec2F Vec2FAdd(Vec2F lhs, Vec2F rhs) {
	return (Vec2F) {lhs.x + rhs.x, lhs.y + rhs.y};
}

Vec2F Vec2FSub(Vec2F lhs, Vec2F rhs) {
	return (Vec2F) {lhs.x - rhs.x, lhs.y - rhs.y};
}

Vec2F Vec2FMul(Vec2F lhs, float rhs) {
	return (Vec2F) {lhs.x * rhs, lhs.y * rhs};
}

float Vec2FLength(Vec2F in) {
	if (in.x == 0.0f && in.y == 0.0f) {
		return 0.0f;
	}
	return sqrtf(powf(in.x, 2.0f) + powf(in.y, 2.0f));
}

Vec2F Vec2FNormalize(Vec2F in) {
	if (in.x == 0.0f && in.y == 0.0f) {
		return in;
	}
	float len = Vec2FLength(in);
	return (Vec2F) { in.x / len, in.y / len };
}

float Vec2FDistance(Vec2F lhs, Vec2F rhs) {
	return Vec2FLength(Vec2FSub(lhs, rhs));
}

float Vec2FAngleRads(Vec2F vector) {
	return atan2f(vector.y, vector.x);
}

bool Vec2FEquals(Vec2F lhs, Vec2F rhs) {
	return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

Vec2I Vec2FTo2I(Vec2F v) {
	return (Vec2I) {(int32_t) roundf(v.x), (int32_t) roundf(v.y)};
}

Vec2F Vec2FFromVec2I(Vec2I v) {
	return (Vec2F) { (float)v.x, (float)v.y };
}
