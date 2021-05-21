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

Vec2I Vec2Fto2I(Vec2F v) {
	return (Vec2I) {(int32_t) round(v.x), (int32_t) round(v.y)};
}
