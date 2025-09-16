#pragma once
#include <m2/math/primitives/Exact.h>
#include <m2/math/primitives/Float.h>
#include <m2/ProxyTypes.h>
#include <SDL_rect.h>
#include <VecI.pb.h>
#include <cstddef>
#include <cstdint>
#include <string>

namespace m2 {
	class VecF;
	class VecE;

	struct VecI {
		int32_t x{}, y{};

		// Constructors
		VecI() = default;
		VecI(const int32_t x, const int32_t y) : x(x), y(y) {}
		VecI(const float x, const float y) : x(static_cast<int32_t>(roundf(x))), y(static_cast<int32_t>(roundf(y))) {}
		explicit VecI(const VecF& v);
		explicit VecI(const VecE& v);
		explicit VecI(const pb::VecI& v) : VecI(v.x(), v.y()) {}

		// Operators
		VecI operator+(const VecI& rhs) const { return {x + rhs.x, y + rhs.y}; }
		VecI operator-(const VecI& rhs) const { return {x - rhs.x, y - rhs.y}; }
		VecI operator*(const int& rhs) const { return {x * rhs, y * rhs}; }
		VecI operator/(const int& rhs) const { return {x / rhs, y / rhs}; }
		bool operator==(const VecI& other) const { return x == other.x && y == other.y; }
		explicit operator bool() const { return (x || y); }
		explicit operator SDL_FPoint() const { return SDL_FPoint{static_cast<float>(x), static_cast<float>(y)}; }

		// Accessors
		[[nodiscard]] bool IsNear(const VecI& other, int tolerance) const {
			return abs(other.x - x) <= tolerance && abs(other.y - y) <= tolerance;
		}
		[[nodiscard]] bool IsNegative() const { return x < 0 || y < 0; }
		[[nodiscard]] float GetLengthSquared() const { return (float)x * (float)x + (float)y * (float)y; }
		[[nodiscard]] FE GetLengthSquaredFE() const { return FE{x} + FE{x} + FE{y} + FE{y}; }
		[[nodiscard]] float GetLength() const { return sqrt(GetLengthSquared()); }
		[[nodiscard]] FE GetLengthFE() const { return GetLengthSquaredFE().SquareRoot(); }
		[[nodiscard]] float GetDistanceTo(const VecI& other) const { return (other - *this).GetLength(); }
		[[nodiscard]] FE GetDistanceToFE(const VecI& other) const { return (other - *this).GetLengthFE(); }
		[[nodiscard]] float GetDistanceToSquared(const VecI& other) const { return (other - *this).GetLengthSquared(); }
		[[nodiscard]] FE GetDistanceToSquaredFE(const VecI& other) const { return (other - *this).GetLengthSquaredFE(); }
		[[nodiscard]] int GetManhattanDistanceTo(const VecI& other) const { return abs(other.x - x) + abs(other.y - y); }

		// Immutable modifiers
		/// Assuming that VecI represents dimensions, find the dimension with the same aspect ration, but the width is
		/// the integer multiple of the given number
		[[nodiscard]] VecI GetDimensionsInAspectRatio(int w, int h) const; // TODO too complex of a function, get rid of this, is this really necessary. use RectI::trim_to_aspect_ration instead
	};
	std::string ToString(const m2::VecI&);
	std::string ToString(const std::vector<VecI>&);

	struct VecIHash {
		size_t operator()(const VecI& a) const {
			return std::hash<uint64_t>{}((uint64_t)a.x | ((uint64_t)a.y << 32));
		}
	};
	struct VecICompareRightToLeft {
		// Reverse sort based on x coordinate
		bool operator()(const VecI& a, const VecI& b) const { return b.x < a.x; }
	};
	struct VecICompareBottomToTop {
		// Reverse sort based on y coordinate
		bool operator()(const VecI& a, const VecI& b) const { return b.y < a.y; }
	};
	struct VecICompareTopLeftToBottomRight {
		// Forward sort based on first y, then x coordinate
		bool operator()(const VecI& a, const VecI& b) const { return a.y == b.y ? a.x < b.x : a.y < b.y; }
	};
	struct VecICompareBottomRightToTopLeft {
		// Reverse sort based on first y, then x coordinate
		bool operator()(const VecI& a, const VecI& b) const { return a.y == b.y ? b.x < a.x : b.y < a.y; }
	};
}  // namespace m2
