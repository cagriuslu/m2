#pragma once

#include <cmath>

namespace m3 {
	struct VecF {
		float x{}, y{}, z{};

		inline VecF() = default;
		inline VecF(float x, float y, float z) : x(x), y(y), z(z) {}
		inline VecF(int x, int y, int z) : x((float)x), y((float)y), z((float)z) {}

		inline VecF operator+(const VecF& rhs) const { return {x + rhs.x, y + rhs.y, z + rhs.z}; }
		inline VecF operator-(const VecF& rhs) const { return {x - rhs.x, y - rhs.y, z - rhs.z}; }

		[[nodiscard]] inline float dot(const VecF& other) const { return x * other.x + y * other.y + z * other.z; }
		[[nodiscard]] inline float length_sq() const { return x * x + y * y + z * z; }
		[[nodiscard]] inline float length() const { return sqrtf(length_sq()); }

		[[nodiscard]] inline VecF offset_x(float amount) const { return {x + amount, y, z}; }
		[[nodiscard]] inline VecF offset_y(float amount) const { return {x, y + amount, z}; }
		[[nodiscard]] inline VecF offset_z(float amount) const { return {x, y, z + amount}; }
		[[nodiscard]] VecF rotate_xy(float rads) const;
		[[nodiscard]] VecF rotate_xz(float rads) const;
	};
}

namespace m2 {
	std::string to_string(const m3::VecF&);
}
