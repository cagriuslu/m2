#ifndef M2_VEC2F_H
#define M2_VEC2F_H

#include <Dim2f.pb.h>
#include <Vec2f.pb.h>
#include <Vec2i.pb.h>
#include <box2d/b2_math.h>
#include <string>
#include <array>

namespace m2 {
	struct Vec2i;

	struct Vec2f {
		float x{}, y{};

		inline Vec2f() = default;
		inline Vec2f(float x, float y) : x(x), y(y) {}
		inline Vec2f(int x, int y) : x((float)x), y((float)y) {}
		inline Vec2f(unsigned x, unsigned y) : x((float)x), y((float)y) {}
		inline explicit Vec2f(const b2Vec2& v) : Vec2f(v.x, v.y) {}
		inline explicit Vec2f(const pb::Dim2f& v) : Vec2f(v.w(), v.h()) {}
		inline explicit Vec2f(const pb::Vec2f& v) : Vec2f(v.x(), v.y()) {}
		inline explicit Vec2f(const pb::Vec2i& v) : Vec2f(v.x(), v.y()) {}

		inline Vec2f operator+(const Vec2f& rhs) const { return {x + rhs.x, y + rhs.y}; }
		inline Vec2f operator+(const float& rhs) const { return {x + rhs, y + rhs}; }
		inline Vec2f& operator+=(const Vec2f& rhs) { x += rhs.x; y += rhs.y; return *this; }
		inline Vec2f operator-(const Vec2f& rhs) const { return {x - rhs.x, y - rhs.y}; }
		inline Vec2f operator-(const float& rhs) const { return {x - rhs, y - rhs}; }
		inline Vec2f operator*(const float& rhs) const { return {x * rhs, y * rhs}; }
		inline Vec2f operator/(const float& rhs) const { return {x / rhs, y / rhs}; }
		inline Vec2f operator/(const int& rhs) const { return {x / (float)rhs, y / (float)rhs}; }
		inline Vec2f operator/(const unsigned& rhs) const { return {x / (float)rhs, y / (float)rhs}; }
		inline bool operator==(const Vec2f& other) const { return (x == other.x) && (y == other.y); }
		inline explicit operator bool() const { return (x != 0.0f) || (y != 0.0f); }
		inline explicit operator b2Vec2() const { return b2Vec2{x, y}; }

		[[nodiscard]] inline bool is_nan() const { return isnan(x) || isnan(y); }
		[[nodiscard]] inline bool is_near(const Vec2f& other, float tolerance) const { return fabsf(other.x - x) <= tolerance && fabsf(other.y - y) <= tolerance; }
		[[nodiscard]] inline bool is_small(float tolerance) const { return is_near({}, tolerance); }
		[[nodiscard]] inline float length_sq() const { return x * x + y * y; }
		[[nodiscard]] inline float length() const { return sqrt(length_sq()); }
		[[nodiscard]] inline float distance_sq(const Vec2f& other) const { return (other - *this).length_sq(); }
		[[nodiscard]] float distance_sq(const Vec2i& other) const;
		[[nodiscard]] inline float distance(const Vec2f& other) const { return (other - *this).length(); }
		[[nodiscard]] inline float angle_rads() const { return atan2f(y, x); }

		[[nodiscard]] inline Vec2f normalize() const { float len = length(); return len != 0.0f ? Vec2f{x / len, y / len} : Vec2f{}; }
		[[nodiscard]] inline Vec2f ln() const { return {logf(x), logf(y)}; }
		[[nodiscard]] inline Vec2f round() const { return {roundf(x), roundf(y)}; }
		[[nodiscard]] inline Vec2f floor() const { return {floorf(x), floorf(y)}; }
		[[nodiscard]] inline Vec2f ceil() const { return {ceilf(x), ceilf(y)}; }
		[[nodiscard]] inline Vec2f with_length(float len) const { return normalize() * len; }
		[[nodiscard]] inline Vec2f floor_length(float len) const { return length() < len ? with_length(len) : *this; }
		[[nodiscard]] inline Vec2f ceil_length(float len) const { return len < length() ? with_length(len) : *this; }
		[[nodiscard]] inline Vec2f lerp(const Vec2f& to, float ratio) const { return *this + (to - *this) * ratio; }
		[[nodiscard]] inline Vec2f rotate(float rads) const { return from_angle(angle_rads() + rads).with_length(length()); }

		/// Order of corners: Bottom-right, Bottom-left, Top-left, Top-right
		[[nodiscard]] std::array<Vec2f, 4> aabb_corners(float aabb_radius) const;

		// Vec2i
		explicit Vec2f(const Vec2i& v);
		[[nodiscard]] Vec2i iround() const;

		inline static Vec2f nan() { return {NAN, NAN}; }
		inline static Vec2f from_angle(float rads) { return {cosf(rads), sinf(rads)}; }
	};

	std::string to_string(const m2::Vec2f&);
}

#endif //M2_VEC2F_H
