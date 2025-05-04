#pragma once
#include <Dim2f.pb.h>
#include <SDL_rect.h>
#include <VecF.pb.h>
#include <VecI.pb.h>
#include <box2d/b2_math.h>

#include <array>
#include <string>
#include <optional>

namespace m2 {
	struct VecI;

	struct VecF {
		float x{}, y{};

		// Constructors

		VecF() = default;
		VecF(float x, float y) : x(x), y(y) {}
		VecF(int x, int y) : x((float)x), y((float)y) {}
		VecF(unsigned x, unsigned y) : x((float)x), y((float)y) {}
		explicit VecF(const VecI& v);
		explicit VecF(const b2Vec2& v) : VecF(v.x, v.y) {}
		explicit VecF(const pb::Dim2f& v) : VecF(v.w(), v.h()) {}
		explicit VecF(const pb::VecF& v) : VecF(v.x(), v.y()) {}
		explicit VecF(const pb::VecI& v) : VecF(v.x(), v.y()) {}

		// Operators

		VecF operator+(const VecF& rhs) const { return {x + rhs.x, y + rhs.y}; }
		VecF operator+(const float& rhs) const { return {x + rhs, y + rhs}; }
		VecF& operator+=(const VecF& rhs) { *this = *this + rhs; return *this; } // TODO make in-place
		VecF operator-(const VecF& rhs) const { return {x - rhs.x, y - rhs.y}; }
		VecF operator-(const float& rhs) const { return {x - rhs, y - rhs}; }
		VecF operator*(const int& rhs) const { return {x * static_cast<float>(rhs), y * static_cast<float>(rhs)}; }
		VecF operator*(const float& rhs) const { return {x * rhs, y * rhs}; }
		VecF operator/(const float& rhs) const { return {x / rhs, y / rhs}; }
		VecF operator/(const int& rhs) const { return {x / static_cast<float>(rhs), y / static_cast<float>(rhs)}; }
		VecF operator/(const unsigned& rhs) const { return {x / static_cast<float>(rhs), y / static_cast<float>(rhs)}; }
		bool operator==(const VecF& other) const { return (x == other.x) && (y == other.y); } // TODO use equals(other, tolerance) instead
		explicit operator bool() const { return (x != 0.0f) || (y != 0.0f); }
		explicit operator b2Vec2() const { return b2Vec2{x, y}; }
		explicit operator pb::VecF() const { pb::VecF v; v.set_x(x); v.set_y(y); return v; }
		explicit operator SDL_FPoint() const { return SDL_FPoint{x, y}; }

		// Accessors

		[[nodiscard]] bool is_nan() const { return isnan(x) || isnan(y); }
		[[nodiscard]] bool is_near(const VecF& other, float tolerance) const {
			return fabsf(other.x - x) <= tolerance && fabsf(other.y - y) <= tolerance;
		}
		[[nodiscard]] bool is_negative() const { return x < 0.0f || y < 0.0f; }
		[[nodiscard]] bool is_small(float tolerance) const { return is_near({}, tolerance); }
		[[nodiscard]] float length_sq() const { return x * x + y * y; }
		[[nodiscard]] float length() const { return sqrtf(length_sq()); }
		[[nodiscard]] float distance_sq(const VecF& other) const { return (other - *this).length_sq(); }
		[[nodiscard]] float distance_sq(const VecI& other) const;
		[[nodiscard]] float distance(const VecF& other) const { return (other - *this).length(); }
		[[nodiscard]] float angle_rads() const { return atan2f(y, x); }

		// Modifiers

		[[nodiscard]] VecF normalize() const {
			float len = length();
			return len != 0.0f ? VecF{x / len, y / len} : VecF{};
		}
		[[nodiscard]] VecF ln() const { return {logf(x), logf(y)}; }
		[[nodiscard]] VecF round() const { return {roundf(x), roundf(y)}; }
		[[nodiscard]] VecF RoundToBin(int binCount) const;
		[[nodiscard]] VecF floor() const { return {floorf(x), floorf(y)}; }
		[[nodiscard]] VecF ceil() const { return {ceilf(x), ceilf(y)}; }
		[[nodiscard]] VecF with_length(float len) const { return normalize() * len; }
		[[nodiscard]] VecF floor_length(float len) const { return length() < len ? with_length(len) : *this; }
		[[nodiscard]] VecF ceil_length(float len) const { return len < length() ? with_length(len) : *this; }
		[[nodiscard]] VecF lerp(const VecF& to, float ratio) const { return *this + (to - *this) * ratio; }
		[[nodiscard]] VecF rotate(float rads) const { return CreateUnitVectorWithAngle(angle_rads() + rads).with_length(length()); }
		[[nodiscard]] VecF clamp(const std::optional<VecF>& min, const std::optional<VecF>& max) const;

		/// Order of corners: Bottom-right, Bottom-left, Top-left, Top-right
		[[nodiscard]] std::array<VecF, 4> aabb_corners(float aabb_radius) const;

		[[nodiscard]] VecI iround() const;
		[[nodiscard]] VecF hround() const;  // Round to halves (ex. 0.0, 0.5, 1.0, 1.5, ...)

		static VecF nan() { return {NAN, NAN}; }
		static VecF CreateUnitVectorWithAngle(float rads) { return {cosf(rads), sinf(rads)}; }
	};

	std::string ToString(const VecF&);

	struct VecFCompareRightToLeft {
		// Reverse sort based on x coordinate
		bool operator()(const VecF& a, const VecF& b) const { return b.x < a.x; }
	};
	struct VecFCompareBottomToTop {
		// Reverse sort based on y coordinate
		bool operator()(const VecF& a, const VecF& b) const { return b.y < a.y; }
	};
	struct VecFCompareTopLeftToBottomRight {
		// Forward sort based on first y, then x coordinate
		bool operator()(const VecF& a, const VecF& b) const { return a.y == b.y ? a.x < b.x : a.y < b.y; }
	};
	struct VecFCompareBottomRightToTopLeft {
		// Reverse sort based on first y, then x coordinate
		bool operator()(const VecF& a, const VecF& b) const { return a.y == b.y ? b.x < a.x : b.y < a.y; }
	};
}  // namespace m2
