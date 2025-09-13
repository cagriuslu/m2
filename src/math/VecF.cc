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

	return VecF{h(_x), h(_y)};
}

float m2::VecF::GetDistanceToSquared(const VecI& other) const {
	return this->GetDistanceToSquared(VecF{other});
}

m2::VecF m2::VecF::RoundToBin(const int binCount) const {
	return {m2::RoundToBin(_x, binCount), m2::RoundToBin(_y, binCount)};
}
m2::VecF m2::VecF::Clamp(const std::optional<VecF>& min, const std::optional<VecF>& max) const {
	auto cp = *this;
	if (min) {
		if (cp.GetX() < min->_x) {
			cp = {min->_x, cp.GetY()};
		}
		if (cp.GetY() < min->GetY()) {
			cp = cp.WithY(min->GetY());
		}
	}
	if (max) {
		if (max->_x < cp.GetX()) {
			cp = {max->_x, cp.GetY()};
		}
		if (max->GetY() < cp.GetY()) {
			cp = cp.WithY(max->GetY());
		}
	}
	return cp;
}
m2::VecF m2::VecF::MoveTowards(const VecF& direction, float distance) const {
	const auto normalizedDirection = direction.Normalize();
	return *this + normalizedDirection * distance;
}

std::array<m2::VecF, 4> m2::VecF::GetAabbCorners(float aabb_radius) const {
	return {VecF{_x + aabb_radius, _y + aabb_radius}, VecF{_x - aabb_radius, _y + aabb_radius}, VecF{_x - aabb_radius, _y - aabb_radius}, VecF{_x + aabb_radius, _y - aabb_radius}};
}

m3::VecF m3::VecF::rotate_xy(float rads) const {
	// Use m2::VecF to do the actual calculation
	auto xy = m2::VecF{x, y};
	auto xy_rotated = xy.Rotate(rads);
	return {xy_rotated.GetX(), xy_rotated.GetY(), z};
}

m3::VecF m3::VecF::rotate_xz(float rads) const {
	// Use m2::VecF to do the actual calculation
	auto xz = m2::VecF{x, z};
	auto xz_rotated = xz.Rotate(rads);
	return {xz_rotated.GetX(), y, xz_rotated.GetY()};
}

std::string m2::ToString(const m2::VecF& v) {
	std::stringstream ss;
	ss << "{x:" << v.GetX() << ",y:" << v.GetY() << "}";
	return ss.str();
}

std::string m2::ToString(const m3::VecF& v) {
	std::stringstream ss;
	ss << "{x:" << v.x << ",y:" << v.y << ",z:" << v.z << "}";
	return ss.str();
}
