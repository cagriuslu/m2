#include <m2/Vec2f.h>
#include <m2/Vec2i.h>
#include <cmath>
#include <sstream>
#include <iomanip>

m2::Vec2f::Vec2f(const Vec2i& v) : Vec2f(v.x, v.y) {}
m2::Vec2i m2::Vec2f::iround() const {
	return Vec2i{this->round()};
}
m2::Vec2i m2::Vec2f::ifloor() const {
	return Vec2i{this->floor()};
}
m2::Vec2i m2::Vec2f::iceil() const {
	return Vec2i{this->ceil()};
}

std::array<m2::Vec2f, 4> m2::Vec2f::aabb_corners(float aabb_radius) const {
	return {Vec2f{x + aabb_radius, y + aabb_radius}, Vec2f{x - aabb_radius, y + aabb_radius}, Vec2f{x - aabb_radius, y - aabb_radius}, Vec2f{x + aabb_radius, y - aabb_radius}};
}

std::string m2::to_string(const m2::Vec2f& v) {
	std::stringstream ss;
	ss << "{x:" << v.x << ",y:" << v.y << "}";
	return ss.str();
}
