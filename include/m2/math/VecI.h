#pragma once
#include <SDL_rect.h>
#include <VecI.pb.h>

#include <cstddef>
#include <cstdint>
#include <string>

namespace m2 {
	struct VecF;

	struct VecI {
		int32_t x{}, y{};

		// Constructors
		inline VecI() = default;
		inline VecI(int32_t x, int32_t y) : x(x), y(y) {}
		inline VecI(float x, float y) : x((int32_t)roundf(x)), y((int32_t)roundf(y)) {}
		explicit VecI(const VecF& v);
		inline explicit VecI(const pb::VecI& v) : VecI(v.x(), v.y()) {}

		// Operators
		inline VecI operator+(const VecI& rhs) const { return {x + rhs.x, y + rhs.y}; }
		inline VecI operator-(const VecI& rhs) const { return {x - rhs.x, y - rhs.y}; }
		inline VecI operator*(const int& rhs) const { return {x * rhs, y * rhs}; }
		inline VecI operator/(const int& rhs) const { return {x / rhs, y / rhs}; }
		inline bool operator==(const VecI& other) const { return x == other.x && y == other.y; }
		inline explicit operator bool() const { return (x || y); }
		inline explicit operator SDL_FPoint() const { return SDL_FPoint{static_cast<float>(x), static_cast<float>(y)}; }

		// Accessors
		[[nodiscard]] inline bool IsNear(const VecI& other, int tolerance) const {
			return abs(other.x - x) <= tolerance && abs(other.y - y) <= tolerance;
		}
		[[nodiscard]] inline bool IsNegative() const { return x < 0 || y < 0; }
		[[nodiscard]] inline float GetLengthSquared() const { return (float)x * (float)x + (float)y * (float)y; }
		[[nodiscard]] inline float GetLength() const { return sqrt(GetLengthSquared()); }
		[[nodiscard]] inline float GetDistanceTo(const VecI& other) const { return (other - *this).GetLength(); }
		[[nodiscard]] inline float GetDistanceToSquared(const VecI& other) const { return (other - *this).GetLengthSquared(); }
		[[nodiscard]] inline int GetManhattanDistanceTo(const VecI& other) const { return abs(other.x - x) + abs(other.y - y); }

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
