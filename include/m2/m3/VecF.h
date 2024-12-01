#pragma once

#include "../math/VecF.h"
#include <VecF.pb.h>

namespace m3 {
	struct VecF {
		float x{}, y{}, z{};

		VecF() = default;
		VecF(const float x, const float y, const float z) : x(x), y(y), z(z) {}
		VecF(const int x, const int y, const int z) : x(static_cast<float>(x)), y(static_cast<float>(y)), z(static_cast<float>(z)) {}
		explicit VecF(const m2::VecF& xy, const float z = 0.0f) : x(xy.x), y(xy.y), z(z) {}
		explicit VecF(const m2::pb::VecF& xy, const float z = 0.0f) : x(xy.x()), y(xy.y()), z(z) {}

		VecF operator+(const VecF& rhs) const { return {x + rhs.x, y + rhs.y, z + rhs.z}; }
		VecF operator-(const VecF& rhs) const { return {x - rhs.x, y - rhs.y, z - rhs.z}; }
		bool operator==(const VecF& other) const { return (x == other.x) && (y == other.y) && (z == other.z); }

		/// Sine of the angle with respect to XY-plane
		[[nodiscard]] float sin_xy() const { return z / length(); }
		/// Cosine of the angle with respect to XY-plane
		[[nodiscard]] float cos_xy() const { return length_xy() / length(); }
		[[nodiscard]] float dot(const VecF& other) const { return x * other.x + y * other.y + z * other.z; }
		[[nodiscard]] float length_sq() const { return x * x + y * y + z * z; }
		[[nodiscard]] float length() const { return sqrtf(length_sq()); }
		/// Length of the projection on XY-plane
		[[nodiscard]] float length_xy() const { return sqrtf(x * x + y * y); }

		[[nodiscard]] VecF offset_x(const float amount) const { return {x + amount, y, z}; }
		[[nodiscard]] VecF offset_y(const float amount) const { return {x, y + amount, z}; }
		[[nodiscard]] VecF offset_z(const float amount) const { return {x, y, z + amount}; }
		[[nodiscard]] VecF rotate_xy(float rads) const;
		[[nodiscard]] VecF rotate_xz(float rads) const;
	};
}

namespace m2 {
	std::string to_string(const m3::VecF&);
}
