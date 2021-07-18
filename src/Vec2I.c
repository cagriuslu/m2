#include "Vec2I.h"

Vec2I Vec2I_Add(Vec2I lhs, Vec2I rhs) {
	return (Vec2I) {lhs.x + rhs.x, lhs.y + rhs.y};
}

bool Vec2I_Equals(Vec2I lhs, Vec2I rhs) {
	return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}
