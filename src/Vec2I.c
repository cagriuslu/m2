#include "m2/Vec2I.h"
#include "m2/Vec2F.h"
#include <math.h>

Vec2I Vec2I_Add(Vec2I lhs, Vec2I rhs) {
	return (Vec2I) {lhs.x + rhs.x, lhs.y + rhs.y};
}

Vec2I Vec2I_Sub(Vec2I lhs, Vec2I rhs) {
	return (Vec2I) {lhs.x - rhs.x, lhs.y - rhs.y};
}

bool Vec2I_Equals(Vec2I lhs, Vec2I rhs) {
	return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

Vec2I Vec2I_From2F(Vec2F v) {
	return (Vec2I) {(int32_t) roundf(v.x), (int32_t) roundf(v.y)};
}
