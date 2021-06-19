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

Vec2I Vec2Fto2I(Vec2F v) {
	return (Vec2I) {(int32_t) round(v.x), (int32_t) round(v.y)};
}
