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
		static RectI CreateCenteredAround(const VecI& center, int w, int h) { return {center.x - w / 2, center.y - h / 2, w, h}; }
		static RectI CreateFromCorners(const VecI& corner1, const VecI& corner2);
		static RectI CreateFromIntersectingCells(const RectF& rect);

		// Operators

		bool operator==(const RectI& other) const;
		explicit operator bool() const;
		explicit operator SDL_Rect() const;
		explicit operator SDL_FRect() const;

		// Accessors

		[[nodiscard]] VecI GetDimensions() const { return {w, h}; }
		[[nodiscard]] bool DoesContain(const VecI&) const;
		/// Beware, X2 will be **past** the rectangle.
		[[nodiscard]] int GetX2() const { return x + w; }
		/// Beware, Y2 will be **past** the rectangle.
		[[nodiscard]] int GetY2() const { return y + h; }
		[[nodiscard]] int GetXCenter() const { return x + w / 2; }
		[[nodiscard]] int GetYCenter() const { return y + h / 2; }
		void ForEachCell(const std::function<void(const VecI&)>& op) const;
		void ForDifference(const RectI& new_rect, const std::function<void(const VecI&)>& on_addition,
		    const std::function<void(const VecI&)>& on_removal) const;
		[[nodiscard]] VecI GetTopLeftPoint() const { return {x, y}; }
		/// Beware, TopRight will be **past** the rectangle.
		[[nodiscard]] VecI GetTopRightPoint() const { return {GetX2(), y}; }
		/// Beware, BottomLeft will be **past** the rectangle.
		[[nodiscard]] VecI GetBottomLeftPoint() const { return {x, GetY2()}; }
		/// Beware, BottomRight will be **past** the rectangle.
		[[nodiscard]] VecI GetBottomRightPoint() const { return {GetX2(), GetY2()}; }
		[[nodiscard]] VecI GetCenterPoint() const { return {GetXCenter(), GetYCenter()}; }

		// Immutable modifiers

		[[nodiscard]] RectI TrimAllSides(int amount) const;
		[[nodiscard]] RectI TrimLeft(int amount) const;
		[[nodiscard]] RectI TrimRight(int amount) const;
		[[nodiscard]] RectI TrimTop(int amount) const;
		[[nodiscard]] RectI TrimBottom(int amount) const;
		[[nodiscard]] RectI TrimToSquare() const;
		[[nodiscard]] RectI TrimToAspectRatio(int w, int h) const; // Result is not exact, but a close approximation
		[[nodiscard]] RectI AlignLeftTo(int _x) const;
		[[nodiscard]] RectI AlignRightTo(int _x) const;
		[[nodiscard]] RectI AlignTopTo(int _y) const;
		[[nodiscard]] RectI AlignBottomTo(int _y) const;
		[[nodiscard]] RectI AlignCenterTo(int _x, int _y) const;
		[[nodiscard]] RectI ExpandAllSides(int amount) const;
		[[nodiscard]] RectI GetRow(int totalRowCount, int rowIndex) const;
		[[nodiscard]] std::optional<RectI> GetIntersection(const RectI& other) const;
		[[nodiscard]] RectI ApplyRatio(const RectF& ratio_rect) const;
	};

	std::string ToString(const RectI&);
}  // namespace m2
