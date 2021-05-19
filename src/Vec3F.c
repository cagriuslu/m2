#include "Vec3F.h"
#include <math.h>

Vec3F Vec3FAdd(Vec3F lhs, Vec3F rhs) {
	return (Vec3F) {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Vec3F Vec3FSub(Vec3F lhs, Vec3F rhs) {
	return (Vec3F) {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

Vec3F Vec3FMul(Vec3F lhs, float rhs) {
	return (Vec3F) {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
}

Vec2I Vec3Fto2I(Vec3F v) {
	return (Vec2I) {(int32_t) round(v.x), (int32_t) round(v.y)};
}
