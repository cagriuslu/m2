#include <m2/Aabb2f.h>

m2::Aabb2f::Aabb2f() : top_left(), bottom_right() {}

m2::Aabb2f::Aabb2f(const Vec2f& top_left, const Vec2f& bottom_right) : top_left(top_left), bottom_right(bottom_right) {}

m2::Aabb2f::Aabb2f(const Vec2i& top_left, const Vec2i& bottom_right) : top_left(top_left), bottom_right(bottom_right) {}

m2::Aabb2f::Aabb2f(const Vec2f& center, float side) : Aabb2f(center, side, side) {}

m2::Aabb2f::Aabb2f(const Vec2f& center, float width, float height) : top_left(center.x - width / 2.0f, center.y - height / 2.0f),
    bottom_right(center.x + width / 2.0f, center.y + height / 2.0f) {}

float m2::Aabb2f::width() const {
	return bottom_right.x - top_left.x;
}

float m2::Aabb2f::height() const {
	return bottom_right.y - top_left.y;
}

m2::Vec2f m2::Aabb2f::center() const {
	return {(top_left.x + bottom_right.x) / 2.0f, (top_left.y + bottom_right.y) / 2.0f};
}

m2::Aabb2f::operator bool() const {
	return (0.0f < width()) || (0.0f < height());
}

m2::Aabb2f::operator b2AABB() const {
    return b2AABB{.lowerBound = b2Vec2{top_left}, .upperBound{bottom_right}};
}
