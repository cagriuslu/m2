#include <m2/vec2f.hh>
#include <m2/vec2i.hh>
#include <cmath>

m2::vec2f::vec2f() : x(0.0f), y(0.0f) {

}
m2::vec2f::vec2f(float x, float y) : x(x), y(y) {

}
m2::vec2f::vec2f(int x, int y) : x(static_cast<float>(x)), y(static_cast<float>(y)) {

}
m2::vec2f::vec2f(const vec2i& v) : vec2f(v.x, v.y) {

}
m2::vec2f::vec2f(const b2Vec2& v) : vec2f(v.x, v.y) {
	
}

m2::vec2f m2::vec2f::operator+(const vec2f& rhs) const {
	return {x + rhs.x, y + rhs.y};
}
m2::vec2f m2::vec2f::operator-(const vec2f& rhs) const {
	return {x - rhs.x, y - rhs.y};
}
m2::vec2f m2::vec2f::operator*(const float& rhs) const {
	return {x * rhs, y * rhs};
}
m2::vec2f m2::vec2f::operator/(const float& rhs) const {
	return {x / rhs, y / rhs};
}
bool m2::vec2f::operator==(const vec2f& other) const {
	return (x == other.x) && (y == other.y);
}
m2::vec2f::operator bool() const {
	return (x != 0.0f) && (y != 0.0f);
}
m2::vec2f::operator b2Vec2() const {
	return b2Vec2{ x, y };
}

bool m2::vec2f::is_nan() const {
	return isnan(x) || isnan(y);
}
float m2::vec2f::length() const {
	return sqrt(length_sq());
}
float m2::vec2f::length_sq() const {
	return x * x + y * y;
}
float m2::vec2f::distance(const vec2f& other) const {
	return (other - *this).length();
}
float m2::vec2f::distance_sq(const vec2f& other) const {
	return (other - *this).length_sq();
}
float m2::vec2f::angle_rads() const {
	return atan2(y, x);
}

m2::vec2f m2::vec2f::normalize() const {
	float len = length();
	if (len != 0.0f) {
		return {x / len, y / len};
	}
	return {};
}
m2::vec2f m2::vec2f::floor_length(float len) const {
	if (length() < len) {
		return with_length(len);
	} else {
		return *this;
	}
}
m2::vec2f m2::vec2f::with_length(float len) const {
	return normalize() * len;
}
m2::vec2f m2::vec2f::ceil_length(float len) const {
	if (len < length()) {
		return with_length(len);
	} else {
		return *this;
	}
}
m2::vec2f m2::vec2f::lerp(const vec2f& to, float ratio) const {
	return (to - *this) * ratio + *this;
}

m2::vec2f m2::vec2f::nan() {
	return {NAN, NAN};
}
m2::vec2f m2::vec2f::from_angle(float rads) {
	return {cosf(rads), sinf(rads)};
}
