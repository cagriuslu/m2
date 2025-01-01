#include <m2/math/VecF.h>
#include <m2/m3/VecF.h>
#include <m2/math/VecI.h>
#include <sstream>

m2::VecF::VecF(const VecI& v) : VecF(v.x, v.y) {}
m2::VecI m2::VecF::iround() const {
	return VecI{this->round()};
}
m2::VecF m2::VecF::hround() const {
	auto h = [](float f) {
		float diff = f - floorf(f), new_f;
		if (diff < 0.25f) {
			new_f = floorf(f);
		} else if (diff < 0.75f) {
			new_f = floorf(f) + 0.5f;
		} else {
			new_f = floorf(f) + 1.0f;
		}
		return new_f;
	};

	return VecF{h(x), h(y)};
}

float m2::VecF::distance_sq(const VecI& other) const {
	return this->distance_sq(VecF{other});
}

m2::VecF m2::VecF::clamp(const std::optional<VecF>& min, const std::optional<VecF>& max) const {
	auto cp = *this;
	if (min) {
		if (cp.x < min->x) {
			cp.x = min->x;
		}
		if (cp.y < min->y) {
			cp.y = min->y;
		}
	}
	if (max) {
		if (max->x < cp.x) {
			cp.x = max->x;
		}
		if (max->y < cp.y) {
			cp.y = max->y;
		}
	}
	return cp;
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

std::string m2::ToString(const m2::VecF& v) {
	std::stringstream ss;
	ss << "{x:" << v.x << ",y:" << v.y << "}";
	return ss.str();
}

std::string m2::ToString(const m3::VecF& v) {
	std::stringstream ss;
	ss << "{x:" << v.x << ",y:" << v.y << ",z:" << v.z << "}";
	return ss.str();
}
