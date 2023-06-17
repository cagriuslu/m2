#include <m2/VecF.h>
#include <m2/m3/VecF.h>
#include <m2/VecI.h>
#include <sstream>

m2::VecF::VecF(const VecI& v) : VecF(v.x, v.y) {}
m2::VecI m2::VecF::iround() const {
	return VecI{this->round()};
}

float m2::VecF::distance_sq(const VecI& other) const {
	return this->distance_sq(VecF{other});
}

std::array<m2::VecF, 4> m2::VecF::aabb_corners(float aabb_radius) const {
	return {VecF{x + aabb_radius, y + aabb_radius}, VecF{x - aabb_radius, y + aabb_radius}, VecF{x - aabb_radius, y - aabb_radius}, VecF{x + aabb_radius, y - aabb_radius}};
}

m3::VecF m3::VecF::rotate_xy(float rads) const {
	// Use m2::VecF to do the actual calculation
	auto xy = m2::VecF{x, y};
	auto xy_rotated = xy.rotate(rads);
	return {xy_rotated.x, xy_rotated.y, z};
}

m3::VecF m3::VecF::rotate_xz(float rads) const {
	// Use m2::VecF to do the actual calculation
	auto xz = m2::VecF{x, z};
	auto xz_rotated = xz.rotate(rads);
	return {xz_rotated.x, y, xz_rotated.y};
}

std::string m2::to_string(const m2::VecF& v) {
	std::stringstream ss;
	ss << "{x:" << v.x << ",y:" << v.y << "}";
	return ss.str();
}

std::string m2::to_string(const m3::VecF& v) {
	std::stringstream ss;
	ss << "{x:" << v.x << ",y:" << v.y << ",z:" << v.z << "}";
	return ss.str();
}
