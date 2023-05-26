#pragma once
#include "VecI.h"
#include <RectI.pb.h>
#include <SDL2/SDL_rect.h>
#include <string>
#include <optional>

namespace m2 {
	struct RectF;

	struct RectI {
		int x, y, w, h;

		RectI();
		RectI(int x, int y, int w, int h);
		explicit RectI(const RectF& r);
		explicit RectI(const SDL_Rect& r);
		explicit RectI(const pb::RectI& r);

		bool operator==(const RectI& other) const;
		explicit operator bool() const;
		explicit operator SDL_Rect() const;
		explicit operator SDL_FRect() const;
		[[nodiscard]] bool point_in_rect(const VecI&) const;

		[[nodiscard]] RectI trim(int amount) const;
		[[nodiscard]] RectI trim_left(int amount) const;
		[[nodiscard]] RectI trim_right(int amount) const;
		[[nodiscard]] RectI trim_top(int amount) const;
		[[nodiscard]] RectI trim_bottom(int amount) const;
		[[nodiscard]] RectI trim_to_square() const;
		[[nodiscard]] RectI expand(int amount) const;
		[[nodiscard]] std::optional<RectI> intersect(const RectI& other) const;

	};

	std::string to_string(const RectI&);
}
