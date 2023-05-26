#include <m2/Aabb.h>

m2::Aabb::Aabb() : top_left(), bottom_right() {}

m2::Aabb::Aabb(const VecF& top_left, const VecF& bottom_right) : top_left(top_left), bottom_right(bottom_right) {}

m2::Aabb::Aabb(const VecI& top_left, const VecI& bottom_right) : top_left(top_left), bottom_right(bottom_right) {}

m2::Aabb::Aabb(const VecF& center, float side) : Aabb(center, side, side) {}

m2::Aabb::Aabb(const VecF& center, float width, float height) : top_left(center.x - width / 2.0f, center.y - height / 2.0f),
    bottom_right(center.x + width / 2.0f, center.y + height / 2.0f) {}

float m2::Aabb::width() const {
	return bottom_right.x - top_left.x;
}

float m2::Aabb::height() const {
	return bottom_right.y - top_left.y;
}

m2::VecF m2::Aabb::center() const {
	return {(top_left.x + bottom_right.x) / 2.0f, (top_left.y + bottom_right.y) / 2.0f};
}

m2::Aabb::operator bool() const {
	return (0.0f < width()) || (0.0f < height());
}

m2::Aabb::operator b2AABB() const {
    return b2AABB{.lowerBound = b2Vec2{top_left}, .upperBound{bottom_right}};
}
