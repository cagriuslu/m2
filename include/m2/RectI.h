#pragma once
#include "VecI.h"
#include <RectI.pb.h>
#include <SDL2/SDL_rect.h>
#include <string>
#include <optional>
#include <functional>

namespace m2 {
	struct RectF;

	struct RectI {
		int x, y, w, h;

		RectI();
		RectI(int x, int y, int w, int h);
		explicit RectI(const RectF& r);
		explicit RectI(const SDL_Rect& r);
		explicit RectI(const pb::RectI& r);
		static inline RectI centered_around(const VecI& center, int w, int h) { return {center.x - w/2, center.y - h/2, w, h}; }
		static RectI from_corners(const VecI& corner1, const VecI& corner2);
		static RectI from_intersecting_cells(const RectF& rect);

		bool operator==(const RectI& other) const;
		explicit operator bool() const;
		explicit operator SDL_Rect() const;
		explicit operator SDL_FRect() const;
		[[nodiscard]] bool point_in_rect(const VecI&) const;
		[[nodiscard]] int x2() const { return x + w; }
		[[nodiscard]] int y2() const { return y + h; }
		void for_each_cell(const std::function<void(const VecI&)>& op) const;
		void for_difference(const RectI& new_rect, const std::function<void(const VecI&)>& on_addition,
				const std::function<void(const VecI&)>& on_removal) const;

		[[nodiscard]] RectI trim(int amount) const;
		[[nodiscard]] RectI trim_left(int amount) const;
		[[nodiscard]] RectI trim_right(int amount) const;
		[[nodiscard]] RectI trim_top(int amount) const;
		[[nodiscard]] RectI trim_bottom(int amount) const;
		[[nodiscard]] RectI trim_to_square() const;
		[[nodiscard]] RectI expand(int amount) const;
		[[nodiscard]] RectI horizontal_split(int split_count, int piece_idx) const;
		[[nodiscard]] std::optional<RectI> intersect(const RectI& other) const;
		[[nodiscard]] RectI ratio(const RectF& ratio_rect) const;
	};

	std::string to_string(const RectI&);
}
