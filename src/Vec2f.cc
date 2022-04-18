#include <m2/Vec2f.h>
#include <m2/Vec2i.h>
#include <cmath>

m2::Vec2f::Vec2f() : x(0.0f), y(0.0f) {}
m2::Vec2f::Vec2f(float x, float y) : x(x), y(y) {}
m2::Vec2f::Vec2f(int x, int y) : x(static_cast<float>(x)), y(static_cast<float>(y)) {}
m2::Vec2f::Vec2f(unsigned x, unsigned y) : x(static_cast<float>(x)), y(static_cast<float>(y)) {}
m2::Vec2f::Vec2f(const Vec2i& v) : Vec2f(v.x, v.y) {}
m2::Vec2f::Vec2f(const b2Vec2& v) : Vec2f(v.x, v.y) {}

m2::Vec2f m2::Vec2f::operator+(const Vec2f& rhs) const {
	return {x + rhs.x, y + rhs.y};
}
m2::Vec2f m2::Vec2f::operator-(const Vec2f& rhs) const {
	return {x - rhs.x, y - rhs.y};
}
m2::Vec2f m2::Vec2f::operator*(const float& rhs) const {
	return {x * rhs, y * rhs};
}
m2::Vec2f m2::Vec2f::operator/(const float& rhs) const {
	return {x / rhs, y / rhs};
}
bool m2::Vec2f::operator==(const Vec2f& other) const {
	return (x == other.x) && (y == other.y);
}
m2::Vec2f::operator bool() const {
	return (x != 0.0f) && (y != 0.0f);
}
m2::Vec2f::operator b2Vec2() const {
	return b2Vec2{ x, y };
}

bool m2::Vec2f::is_nan() const {
	return isnan(x) || isnan(y);
}
float m2::Vec2f::length() const {
	return sqrt(length_sq());
}
float m2::Vec2f::length_sq() const {
	return x * x + y * y;
}
float m2::Vec2f::distance(const Vec2f& other) const {
	return (other - *this).length();
}
float m2::Vec2f::distance_sq(const Vec2f& other) const {
	return (other - *this).length_sq();
}
float m2::Vec2f::angle_rads() const {
	return atan2(y, x);
}

m2::Vec2f m2::Vec2f::normalize() const {
	float len = length();
	if (len != 0.0f) {
		return {x / len, y / len};
	}
	return {};
}
m2::Vec2f m2::Vec2f::floor_length(float len) const {
	if (length() < len) {
		return with_length(len);
	} else {
		return *this;
	}
}
m2::Vec2f m2::Vec2f::with_length(float len) const {
	return normalize() * len;
}
m2::Vec2f m2::Vec2f::ceil_length(float len) const {
	if (len < length()) {
		return with_length(len);
	} else {
		return *this;
	}
}
m2::Vec2f m2::Vec2f::lerp(const Vec2f& to, float ratio) const {
	return (to - *this) * ratio + *this;
}

m2::Vec2f m2::Vec2f::nan() {
	return {NAN, NAN};
}
m2::Vec2f m2::Vec2f::from_angle(float rads) {
	return {cosf(rads), sinf(rads)};
}
