#include <m2/math/AABB.h>

m2::AABB::AABB() : top_left(), bottom_right() {}

m2::AABB::AABB(const VecF& top_left, const VecF& bottom_right) : top_left(top_left), bottom_right(bottom_right) {}

m2::AABB::AABB(const VecI& top_left, const VecI& bottom_right) : top_left(top_left), bottom_right(bottom_right) {}

m2::AABB::AABB(const VecF& center, float side) : AABB(center, side, side) {}

m2::AABB::AABB(const VecF& center, float width, float height) : top_left(center.GetX() - width / 2.0f, center.GetY() - height / 2.0f),
    bottom_right(center.GetX() + width / 2.0f, center.GetY() + height / 2.0f) {}

float m2::AABB::GetWidth() const {
	return bottom_right.GetX() - top_left.GetX();
}

float m2::AABB::GetHeight() const {
	return bottom_right.GetY() - top_left.GetY();
}

m2::VecF m2::AABB::GetCenterPoint() const {
	return {(top_left.GetX() + bottom_right.GetX()) / 2.0f, (top_left.GetY() + bottom_right.GetY()) / 2.0f};
}

m2::AABB::operator bool() const {
	return (0.0f < GetWidth()) || (0.0f < GetHeight());
}

m2::AABB::operator b2AABB() const {
    return b2AABB{.lowerBound = b2Vec2{top_left}, .upperBound = b2Vec2{bottom_right}};
}
