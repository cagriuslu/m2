#pragma once
#include <RectI.pb.h>
#include <SDL2/SDL_rect.h>

#include <functional>
#include <optional>
#include <string>

#include "VecI.h"

namespace m2 {
	struct RectF;

	struct RectI {
		int x, y, w, h;

		// Constructors

		RectI();
		RectI(int x, int y, int w, int h);
		explicit RectI(const RectF& r);
		explicit RectI(const SDL_Rect& r);
		explicit RectI(const pb::RectI& r);
		static RectI centered_around(const VecI& center, int w, int h) { return {center.x - w / 2, center.y - h / 2, w, h}; }
		static RectI from_corners(const VecI& corner1, const VecI& corner2);
		static RectI from_intersecting_cells(const RectF& rect);

		// Operators

		bool operator==(const RectI& other) const;
		explicit operator bool() const;
		explicit operator SDL_Rect() const;
		explicit operator SDL_FRect() const;

		// Accessors

		[[nodiscard]] VecI dimensions() const { return {w, h}; }
		[[nodiscard]] bool contains(const VecI&) const;
		/// Beware, X2 will be **past** the rectangle.
		[[nodiscard]] int x2() const { return x + w; }
		/// Beware, Y2 will be **past** the rectangle.
		[[nodiscard]] int y2() const { return y + h; }
		[[nodiscard]] int x_center() const { return x + w / 2; }
		[[nodiscard]] int y_center() const { return y + h / 2; }
		void for_each_cell(const std::function<void(const VecI&)>& op) const;
		void for_difference(const RectI& new_rect, const std::function<void(const VecI&)>& on_addition,
		    const std::function<void(const VecI&)>& on_removal) const;
		[[nodiscard]] VecI top_left() const { return {x, y}; }
		/// Beware, TopRight will be **past** the rectangle.
		[[nodiscard]] VecI top_right() const { return {x2(), y}; }
		/// Beware, BottomLeft will be **past** the rectangle.
		[[nodiscard]] VecI bottom_left() const { return {x, y2()}; }
		/// Beware, BottomRight will be **past** the rectangle.
		[[nodiscard]] VecI bottom_right() const { return {x2(), y2()}; }
		[[nodiscard]] VecI center() const { return {x_center(), y_center()}; }

		// Immutable modifiers

		[[nodiscard]] RectI trim(int amount) const;
		[[nodiscard]] RectI trim_left(int amount) const;
		[[nodiscard]] RectI trim_right(int amount) const;
		[[nodiscard]] RectI trim_top(int amount) const;
		[[nodiscard]] RectI trim_bottom(int amount) const;
		[[nodiscard]] RectI trim_to_square() const;
		[[nodiscard]] RectI trim_to_aspect_ratio(int w, int h) const; // Result is not exact, but a close approximation
		[[nodiscard]] RectI align_left_to(int _x) const;
		[[nodiscard]] RectI align_right_to(int _x) const;
		[[nodiscard]] RectI align_top_to(int _y) const;
		[[nodiscard]] RectI align_bottom_to(int _y) const;
		[[nodiscard]] RectI align_center_to(int _x, int _y) const;
		[[nodiscard]] RectI expand(int amount) const;
		[[nodiscard]] RectI horizontal_split(int split_count, int piece_idx) const;
		[[nodiscard]] std::optional<RectI> intersect(const RectI& other) const;
		[[nodiscard]] RectI ratio(const RectF& ratio_rect) const;
	};

	std::string ToString(const RectI&);
}  // namespace m2
