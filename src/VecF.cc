#include <m2/VecF.h>
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

std::string m2::to_string(const m2::VecF& v) {
	std::stringstream ss;
	ss << "{x:" << v.x << ",y:" << v.y << "}";
	return ss.str();
}
