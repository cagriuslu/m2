#include <m2/AABB2f.h>

m2::AABB2f::AABB2f() : top_left(), bottom_right() {}

m2::AABB2f::AABB2f(const Vec2f& top_left, const Vec2f& bottom_right) : top_left(top_left), bottom_right(bottom_right) {}

m2::AABB2f::AABB2f(const Vec2i& top_left, const Vec2i& bottom_right) : top_left(top_left), bottom_right(bottom_right) {}

float m2::AABB2f::width() const {
	return bottom_right.x - top_left.x;
}

float m2::AABB2f::height() const {
	return bottom_right.y - top_left.y;
}

m2::Vec2f m2::AABB2f::center() const {
	return {(top_left.x + bottom_right.x) / 2.0f, (top_left.y + bottom_right.y) / 2.0f};
}

m2::AABB2f::operator bool() const {
	return (0.0f < width()) || (0.0f < height());
}
