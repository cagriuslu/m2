#include "Vec2I.h"

Vec2I Vec2IAdd(Vec2I lhs, Vec2I rhs) {
	return (Vec2I) {lhs.x + rhs.x, lhs.y + rhs.y};
}
