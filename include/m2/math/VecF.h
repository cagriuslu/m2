#pragma once
#include <m2/math/primitives/Exact.h>
#include <m2/math/primitives/Float.h>
#include <m2/ProxyTypes.h>
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

	class VecF {
		float _x{}, _y{};

	public:
		// Constructors

		VecF() = default;
		VecF(float x, float y) : _x(x), _y(y) {}
		VecF(int x, int y) : _x((float)x), _y((float)y) {}
		VecF(unsigned x, unsigned y) : _x((float)x), _y((float)y) {}
		explicit VecF(const VecI& v);
		explicit VecF(const b2Vec2& v) : VecF(v.x, v.y) {}
		explicit VecF(const pb::Dim2f& v) : VecF(v.w(), v.h()) {}
		explicit VecF(const pb::VecF& v) : VecF(v.x(), v.y()) {}
		explicit VecF(const pb::VecI& v) : VecF(v.x(), v.y()) {}

		// Operators

		VecF operator+(const VecF& rhs) const { return {_x + rhs.GetX(), _y + rhs.GetY()}; }
		VecF operator+(const float& rhs) const { return {_x + rhs, _y + rhs}; }
		VecF& operator+=(const VecF& rhs) { *this = *this + rhs; return *this; } // TODO make in-place
		VecF operator-(const VecF& rhs) const { return {_x - rhs.GetX(), _y - rhs.GetY()}; }
		VecF operator-(const float& rhs) const { return {_x - rhs, _y - rhs}; }
		VecF operator*(const int& rhs) const { return {_x * static_cast<float>(rhs), _y * static_cast<float>(rhs)}; }
		VecF operator*(const float& rhs) const { return {_x * rhs, _y * rhs}; }
		VecF operator/(const float& rhs) const { return {_x / rhs, _y / rhs}; }
		VecF operator/(const int& rhs) const { return {_x / static_cast<float>(rhs), _y / static_cast<float>(rhs)}; }
		VecF operator/(const unsigned& rhs) const { return {_x / static_cast<float>(rhs), _y / static_cast<float>(rhs)}; }
		bool operator==(const VecF& other) const { return (_x == other.GetX()) && (_y == other.GetY()); } // TODO use equals(other, tolerance) instead
		explicit operator bool() const { return (_x != 0.0f) || (_y != 0.0f); }
		explicit operator b2Vec2() const { return b2Vec2{_x, _y}; }
		explicit operator pb::VecF() const { pb::VecF v; v.set_x(_x); v.set_y(_y); return v; }
		explicit operator SDL_FPoint() const { return SDL_FPoint{_x, _y}; }

		// Accessors

		[[nodiscard]] float GetX() const { return _x; }
		[[nodiscard]] float GetY() const { return _y; }
		[[nodiscard]] bool IsNan() const { return isnan(_x) || isnan(_y); }
		[[nodiscard]] bool IsNear(const VecF& other, float tolerance) const {
			return fabsf(other.GetX() - _x) <= tolerance && fabsf(other.GetY() - _y) <= tolerance;
		}
		[[nodiscard]] bool IsNegative() const { return _x < 0.0f || _y < 0.0f; }
		[[nodiscard]] bool IsSmall(float tolerance) const { return IsNear({}, tolerance); }
		[[nodiscard]] float GetLengthSquared() const { return _x * _x + _y * _y; }
		[[nodiscard]] Float GetLengthSquaredFE() const { return Float{_x} * Float{_x} + Float{_y} * Float{_y}; }
		[[nodiscard]] float GetLength() const { return sqrtf(GetLengthSquared()); }
		[[nodiscard]] float GetDistanceToSquared(const VecF& other) const { return (other - *this).GetLengthSquared(); }
		[[nodiscard]] Float GetDistanceToSquaredFE(const VecF& other) const { return (other - *this).GetLengthSquaredFE(); }
		[[nodiscard]] float GetDistanceToSquared(const VecI& other) const;
		[[nodiscard]] float GetDistanceTo(const VecF& other) const { return (other - *this).GetLength(); }
		/// Returns the angle in radians between this vector and the positive-x axis. [-PI, PI]
		[[nodiscard]] float GetAngle() const { return atan2f(_y, _x); }
		[[nodiscard]] VecF GetCenterBetween(const VecF& other) const { return {(_x + other.GetX()) / 2.0f, (_y + other.GetY()) / 2.0f}; }

		// Modifiers

		[[nodiscard]] VecF WithX(const float x) const { return {x, _y}; }
		[[nodiscard]] VecF WithY(const float y) const { return {_x, y}; }
		[[nodiscard]] VecF Normalize() const {
			float len = GetLength();
			return len != 0.0f ? VecF{_x / len, _y / len} : VecF{};
		}
		[[nodiscard]] VecF Ln() const { return {logf(_x), logf(_y)}; }
		[[nodiscard]] VecF Round() const { return {roundf(_x), roundf(_y)}; }
		[[nodiscard]] VecF RoundToBin(int binCount) const;
		[[nodiscard]] VecF Floor() const { return {floorf(_x), floorf(_y)}; }
		[[nodiscard]] VecF Ceil() const { return {ceilf(_x), ceilf(_y)}; }
		[[nodiscard]] VecF WithLength(const float len) const { return Normalize() * len; }
		[[nodiscard]] VecF FloorLength(const float len) const { return GetLength() < len ? WithLength(len) : *this; }
		[[nodiscard]] VecF CeilLength(const float len) const { return len < GetLength() ? WithLength(len) : *this; }
		[[nodiscard]] VecF Lerp(const VecF& to, const float ratio) const { return *this + (to - *this) * ratio; }
		/// Rotates the vector around origin
		[[nodiscard]] VecF Rotate(const float rads) const { return CreateUnitVectorWithAngle(GetAngle() + rads).WithLength(GetLength()); }
		[[nodiscard]] VecF Clamp(const std::optional<VecF>& min, const std::optional<VecF>& max) const;
		[[nodiscard]] VecF MoveTowards(const VecF& direction, float distance) const;
		[[nodiscard]] float DotProduct(const VecF& other) const { return _x * other.GetX() + _y * other.GetY(); }

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
		bool operator()(const VecF& a, const VecF& b) const { return b.GetX() < a.GetX(); }
	};
	struct VecFCompareBottomToTop {
		// Reverse sort based on y coordinate
		bool operator()(const VecF& a, const VecF& b) const { return b.GetY() < a.GetY(); }
	};
	struct VecFCompareTopLeftToBottomRight {
		// Forward sort based on first y, then x coordinate
		bool operator()(const VecF& a, const VecF& b) const { return a.GetY() == b.GetY() ? a.GetX() < b.GetX() : a.GetY() < b.GetY(); }
	};
	struct VecFCompareBottomRightToTopLeft {
		// Reverse sort based on first y, then x coordinate
		bool operator()(const VecF& a, const VecF& b) const { return a.GetY() == b.GetY() ? b.GetX() < a.GetX() : b.GetY() < a.GetY(); }
	};
}  // namespace m2
