#pragma once
#include <Dim2f.pb.h>
#include <VecF.pb.h>
#include <VecI.pb.h>
#include <box2d/b2_math.h>
#include <SDL_rect.h>
#include <string>
#include <array>

namespace m2 {
	struct VecI;

	struct VecF {
		float x{}, y{};

		inline VecF() = default;
		inline VecF(float x, float y) : x(x), y(y) {}
		inline VecF(int x, int y) : x((float)x), y((float)y) {}
		inline VecF(unsigned x, unsigned y) : x((float)x), y((float)y) {}
		inline explicit VecF(const b2Vec2& v) : VecF(v.x, v.y) {}
		inline explicit VecF(const pb::Dim2f& v) : VecF(v.w(), v.h()) {}
		inline explicit VecF(const pb::VecF& v) : VecF(v.x(), v.y()) {}
		inline explicit VecF(const pb::VecI& v) : VecF(v.x(), v.y()) {}

		inline VecF operator+(const VecF& rhs) const { return {x + rhs.x, y + rhs.y}; }
		inline VecF operator+(const float& rhs) const { return {x + rhs, y + rhs}; }
		inline VecF& operator+=(const VecF& rhs) { x += rhs.x; y += rhs.y; return *this; }
		inline VecF operator-(const VecF& rhs) const { return {x - rhs.x, y - rhs.y}; }
		inline VecF operator-(const float& rhs) const { return {x - rhs, y - rhs}; }
		inline VecF operator*(const float& rhs) const { return {x * rhs, y * rhs}; }
		inline VecF operator/(const float& rhs) const { return {x / rhs, y / rhs}; }
		inline VecF operator/(const int& rhs) const { return {x / (float)rhs, y / (float)rhs}; }
		inline VecF operator/(const unsigned& rhs) const { return {x / (float)rhs, y / (float)rhs}; }
		inline bool operator==(const VecF& other) const { return (x == other.x) && (y == other.y); }
		inline explicit operator bool() const { return (x != 0.0f) || (y != 0.0f); }
		inline explicit operator b2Vec2() const { return b2Vec2{x, y}; }
		inline explicit operator SDL_FPoint() const { return SDL_FPoint{x, y}; }

		[[nodiscard]] inline bool is_nan() const { return isnan(x) || isnan(y); }
		[[nodiscard]] inline bool is_near(const VecF& other, float tolerance) const { return fabsf(other.x - x) <= tolerance && fabsf(other.y - y) <= tolerance; }
		[[nodiscard]] inline bool is_small(float tolerance) const { return is_near({}, tolerance); }
		[[nodiscard]] inline float length_sq() const { return x * x + y * y; }
		[[nodiscard]] inline float length() const { return sqrtf(length_sq()); }
		[[nodiscard]] inline float distance_sq(const VecF& other) const { return (other - *this).length_sq(); }
		[[nodiscard]] float distance_sq(const VecI& other) const;
		[[nodiscard]] inline float distance(const VecF& other) const { return (other - *this).length(); }
		[[nodiscard]] inline float angle_rads() const { return atan2f(y, x); }

		[[nodiscard]] inline VecF normalize() const { float len = length(); return len != 0.0f ? VecF{x / len, y / len} : VecF{}; }
		[[nodiscard]] inline VecF ln() const { return {logf(x), logf(y)}; }
		[[nodiscard]] inline VecF round() const { return {roundf(x), roundf(y)}; }
		[[nodiscard]] inline VecF floor() const { return {floorf(x), floorf(y)}; }
		[[nodiscard]] inline VecF ceil() const { return {ceilf(x), ceilf(y)}; }
		[[nodiscard]] inline VecF with_length(float len) const { return normalize() * len; }
		[[nodiscard]] inline VecF floor_length(float len) const { return length() < len ? with_length(len) : *this; }
		[[nodiscard]] inline VecF ceil_length(float len) const { return len < length() ? with_length(len) : *this; }
		[[nodiscard]] inline VecF lerp(const VecF& to, float ratio) const { return *this + (to - *this) * ratio; }
		[[nodiscard]] inline VecF rotate(float rads) const { return from_angle(angle_rads() + rads).with_length(length()); }

		/// Order of corners: Bottom-right, Bottom-left, Top-left, Top-right
		[[nodiscard]] std::array<VecF, 4> aabb_corners(float aabb_radius) const;

		// VecI
		explicit VecF(const VecI& v);
		[[nodiscard]] VecI iround() const;

		inline static VecF nan() { return {NAN, NAN}; }
		inline static VecF from_angle(float rads) { return {cosf(rads), sinf(rads)}; }
	};

	std::string to_string(const VecF&);
}
