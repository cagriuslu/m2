#include <m2/AABB.h>

m2::AABB::AABB() : top_left(), bottom_right() {}

m2::AABB::AABB(const VecF& top_left, const VecF& bottom_right) : top_left(top_left), bottom_right(bottom_right) {}

m2::AABB::AABB(const VecI& top_left, const VecI& bottom_right) : top_left(top_left), bottom_right(bottom_right) {}

m2::AABB::AABB(const VecF& center, float side) : AABB(center, side, side) {}

m2::AABB::AABB(const VecF& center, float width, float height) : top_left(center.x - width / 2.0f, center.y - height / 2.0f),
    bottom_right(center.x + width / 2.0f, center.y + height / 2.0f) {}

float m2::AABB::width() const {
	return bottom_right.x - top_left.x;
}

float m2::AABB::height() const {
	return bottom_right.y - top_left.y;
}

m2::VecF m2::AABB::center() const {
	return {(top_left.x + bottom_right.x) / 2.0f, (top_left.y + bottom_right.y) / 2.0f};
}

m2::AABB::operator bool() const {
	return (0.0f < width()) || (0.0f < height());
}

m2::AABB::operator b2AABB() const {
    return b2AABB{.lowerBound = b2Vec2{top_left}, .upperBound{bottom_right}};
}
