#pragma once
#include <VecI.pb.h>
#include <SDL_rect.h>
#include <cstdint>
#include <cstddef>
#include <string>

namespace m2 {
	struct VecF;
	
	struct VecI {
		int32_t x{}, y{};

		inline VecI() = default;
		inline VecI(int32_t x, int32_t y) : x(x), y(y) {}
		inline VecI(float x, float y) : x((int32_t)roundf(x)), y((int32_t)roundf(y)) {}
		explicit VecI(const VecF& v);
		inline explicit VecI(const pb::VecI& v) : VecI(v.x(), v.y()) {}

		inline VecI operator+(const VecI& rhs) const { return {x + rhs.x, y + rhs.y}; }
		inline VecI operator-(const VecI& rhs) const { return {x - rhs.x, y - rhs.y}; }
		inline bool operator==(const VecI& other) const { return x == other.x && y == other.y; }
		inline explicit operator bool() const { return (x || y); }
		inline explicit operator SDL_FPoint() const { return SDL_FPoint{static_cast<float>(x), static_cast<float>(y)}; }

		[[nodiscard]] inline bool is_near(const VecI& other, int tolerance) const { return abs(other.x - x) <= tolerance && abs(other.y - y) <= tolerance; }
		[[nodiscard]] inline bool is_negative() const { return x < 0 || y < 0; }
		[[nodiscard]] inline float length_sq() const { return (float)x * (float)x + (float)y * (float)y; }
		[[nodiscard]] inline float length() const { return sqrt(length_sq()); }
		[[nodiscard]] inline float distance(const VecI& other) const { return (other - *this).length(); }
		[[nodiscard]] inline float distance_sq(const VecI& other) const { return (other - *this).length_sq(); }
		[[nodiscard]] inline int manhattan_distance(const VecI& other) const { return abs(other.x - x) + abs(other.y - y); }

		/// Iterates over the cells in the rectangle between this and the other cell.
		void for_each_cell_in_between(const VecI& other, const std::function<void(const VecI&)>& f) const;
	};
	std::string to_string(const m2::VecI&);
	std::string to_string(const std::vector<VecI>&);

	struct VecIHash {
		inline size_t operator()(const VecI& a) const { return std::hash<uint64_t>{}((uint64_t)a.x | ((uint64_t)a.y << 32)); }
	};
	struct VecICompareRightToLeft {
		// Reverse sort based on x coordinate
		inline bool operator()(const VecI& a, const VecI& b) { return b.x < a.x; }
	};
	struct VecICompareBottomToTop {
		// Reverse sort based on y coordinate
		inline bool operator()(const VecI& a, const VecI& b) { return b.y < a.y; }
	};
	struct VecICompareTopLeftToBottomRight {
		// Forward sort based on first y, then x coordinate
		inline bool operator()(const VecI& a, const VecI& b) { return a.y == b.y ? a.x < b.x : a.y < b.y; }
	};
	struct VecICompareBottomRightToTopLeft {
		// Reverse sort based on first y, then x coordinate
		inline bool operator()(const VecI& a, const VecI& b) { return a.y == b.y ? b.x < a.x : b.y < a.y; }
	};
}
