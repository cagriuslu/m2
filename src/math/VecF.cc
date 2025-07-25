#include <m2/math/VecF.h>
#include <m2/Math.h>
#include <m2/m3/VecF.h>
#include <m2/math/VecI.h>
#include <sstream>

m2::VecF::VecF(const VecI& v) : VecF(v.x, v.y) {}
m2::VecI m2::VecF::RoundI() const {
	return VecI{this->Round()};
}
m2::VecF m2::VecF::RoundHalfI() const {
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

float m2::VecF::GetDistanceToSquared(const VecI& other) const {
	return this->GetDistanceToSquared(VecF{other});
}

m2::VecF m2::VecF::RoundToBin(const int binCount) const {
	return {m2::RoundToBin(x, binCount), m2::RoundToBin(y, binCount)};
}
m2::VecF m2::VecF::Clamp(const std::optional<VecF>& min, const std::optional<VecF>& max) const {
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
m2::VecF m2::VecF::MoveTowards(const VecF& direction, float distance) const {
	const auto normalizedDirection = direction.Normalize();
	return *this + normalizedDirection * distance;
}

std::array<m2::VecF, 4> m2::VecF::GetAabbCorners(float aabb_radius) const {
	return {VecF{x + aabb_radius, y + aabb_radius}, VecF{x - aabb_radius, y + aabb_radius}, VecF{x - aabb_radius, y - aabb_radius}, VecF{x + aabb_radius, y - aabb_radius}};
}

m3::VecF m3::VecF::rotate_xy(float rads) const {
	// Use m2::VecF to do the actual calculation
	auto xy = m2::VecF{x, y};
	auto xy_rotated = xy.Rotate(rads);
	return {xy_rotated.x, xy_rotated.y, z};
}

m3::VecF m3::VecF::rotate_xz(float rads) const {
	// Use m2::VecF to do the actual calculation
	auto xz = m2::VecF{x, z};
	auto xz_rotated = xz.Rotate(rads);
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
