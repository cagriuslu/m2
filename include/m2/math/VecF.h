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

		[[nodiscard]] bool IsNan() const { return isnan(x) || isnan(y); }
		[[nodiscard]] bool IsNear(const VecF& other, float tolerance) const {
			return fabsf(other.x - x) <= tolerance && fabsf(other.y - y) <= tolerance;
		}
		[[nodiscard]] bool IsNegative() const { return x < 0.0f || y < 0.0f; }
		[[nodiscard]] bool IsSmall(float tolerance) const { return IsNear({}, tolerance); }
		[[nodiscard]] float GetLengthSquared() const { return x * x + y * y; }
		[[nodiscard]] float GetLength() const { return sqrtf(GetLengthSquared()); }
		[[nodiscard]] float GetDistanceToSquared(const VecF& other) const { return (other - *this).GetLengthSquared(); }
		[[nodiscard]] float GetDistanceToSquared(const VecI& other) const;
		[[nodiscard]] float GetDistanceTo(const VecF& other) const { return (other - *this).GetLength(); }
		/// Returns the angle in radians between this vector and the positive-x axis. [-PI, PI]
		[[nodiscard]] float GetAngle() const { return atan2f(y, x); }
		[[nodiscard]] VecF GetCenterBetween(const VecF& other) const { return {(x + other.x) / 2.0f, (y + other.y) / 2.0f}; }

		// Modifiers

		[[nodiscard]] VecF Normalize() const {
			float len = GetLength();
			return len != 0.0f ? VecF{x / len, y / len} : VecF{};
		}
		[[nodiscard]] VecF Ln() const { return {logf(x), logf(y)}; }
		[[nodiscard]] VecF Round() const { return {roundf(x), roundf(y)}; }
		[[nodiscard]] VecF RoundToBin(int binCount) const;
		[[nodiscard]] VecF Floor() const { return {floorf(x), floorf(y)}; }
		[[nodiscard]] VecF Ceil() const { return {ceilf(x), ceilf(y)}; }
		[[nodiscard]] VecF WithLength(const float len) const { return Normalize() * len; }
		[[nodiscard]] VecF FloorLength(const float len) const { return GetLength() < len ? WithLength(len) : *this; }
		[[nodiscard]] VecF CeilLength(const float len) const { return len < GetLength() ? WithLength(len) : *this; }
		[[nodiscard]] VecF Lerp(const VecF& to, const float ratio) const { return *this + (to - *this) * ratio; }
		/// Rotates the vector around origin
		[[nodiscard]] VecF Rotate(const float rads) const { return CreateUnitVectorWithAngle(GetAngle() + rads).WithLength(GetLength()); }
		[[nodiscard]] VecF Clamp(const std::optional<VecF>& min, const std::optional<VecF>& max) const;
		[[nodiscard]] VecF MoveTowards(const VecF& direction, float distance) const;
		[[nodiscard]] float DotProduct(const VecF& other) const { return x * other.x + y * other.y; }

		/// Order of corners: Bottom-right, Bottom-left, Top-left, Top-right
		[[nodiscard]] std::array<VecF, 4> GetAabbCorners(float aabb_radius) const;

		[[nodiscard]] VecI RoundI() const;
		[[nodiscard]] VecF RoundHalfI() const;  // Round to halves (ex. 0.0, 0.5, 1.0, 1.5, ...)

		static VecF Nan() { return {NAN, NAN}; }
		static VecF CreateUnitVectorWithAngle(const float rads) { return {cosf(rads), sinf(rads)}; }
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
