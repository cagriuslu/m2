#ifndef M2_VEC2I_H
#define M2_VEC2I_H

#include <Vec2i.pb.h>
#include <cstdint>
#include <cstddef>
#include <string>

namespace m2 {
	struct Vec2f;
	
	struct Vec2i {
		int32_t x{}, y{};

		inline Vec2i() = default;
		inline Vec2i(int32_t x, int32_t y) : x(x), y(y) {}
		inline Vec2i(float x, float y) : x((int32_t)x), y((int32_t)y) {}
		explicit Vec2i(const Vec2f& v);
		inline explicit Vec2i(const pb::Vec2i& v) : Vec2i(v.x(), v.y()) {}

		inline Vec2i operator+(const Vec2i& rhs) const { return {x + rhs.x, y + rhs.y}; }
		inline Vec2i operator-(const Vec2i& rhs) const { return {x - rhs.x, y - rhs.y}; }
		inline bool operator==(const Vec2i& other) const { return x == other.x && y == other.y; }
		inline explicit operator bool() const { return (x || y); }

		[[nodiscard]] inline bool is_near(const Vec2i& other, int tolerance) const { return abs(other.x - x) <= tolerance && abs(other.y - y) <= tolerance; }
		[[nodiscard]] inline bool is_negative() const { return x < 0 || y < 0; }
		[[nodiscard]] inline float length_sq() const { return (float)x * (float)x + (float)y * (float)y; }
		[[nodiscard]] inline float length() const { return sqrt(length_sq()); }
		[[nodiscard]] inline float distance(const Vec2i& other) const { return (other - *this).length(); }
		[[nodiscard]] inline int manhattan_distance(const Vec2i& other) const { return abs(other.x - x) + abs(other.y - y); }

		/// Iterates over the cells in the rectangle between this and the other cell.
		void for_each_cell_in_between(const Vec2i& other, const std::function<void(const Vec2i&)>& f) const;
	};
	std::string to_string(const m2::Vec2i&);

	struct Vec2iHash {
		inline size_t operator()(const Vec2i& a) const { return std::hash<uint64_t>{}((uint64_t)a.x | ((uint64_t)a.y << 32)); }
	};
	struct Vec2iCompareRightToLeft {
		// Reverse sort based on x coordinate
		inline bool operator()(const Vec2i& a, const Vec2i& b) { return b.x < a.x; }
	};
	struct Vec2iCompareBottomToTop {
		// Reverse sort based on y coordinate
		inline bool operator()(const Vec2i& a, const Vec2i& b) { return b.y < a.y; }
	};
	struct Vec2iCompareTopLeftToBottomRight {
		// Forward sort based on first y, then x coordinate
		inline bool operator()(const Vec2i& a, const Vec2i& b) { return a.y == b.y ? a.x < b.x : a.y < b.y; }
	};
	struct Vec2iCompareBottomRightToTopLeft {
		// Reverse sort based on first y, then x coordinate
		inline bool operator()(const Vec2i& a, const Vec2i& b) { return a.y == b.y ? b.x < a.x : b.y < a.y; }
	};
}

#endif //M2_VEC2I_H
