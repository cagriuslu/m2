#pragma once
#include "Vec2i.h"
#include <Rect2i.pb.h>
#include <SDL2/SDL_rect.h>
#include <string>
#include <optional>

namespace m2 {
	struct Rect2f;

	struct Rect2i {
		int x, y, w, h;

		Rect2i();
		Rect2i(int x, int y, int w, int h);
		explicit Rect2i(const Rect2f& r);
		explicit Rect2i(const SDL_Rect& r);
		explicit Rect2i(const pb::Rect2i& r);

		bool operator==(const Rect2i& other) const;
		explicit operator bool() const;
		explicit operator SDL_Rect() const;
		explicit operator SDL_FRect() const;
		[[nodiscard]] bool point_in_rect(const Vec2i&) const;

		[[nodiscard]] Rect2i trim(int amount) const;
		[[nodiscard]] Rect2i trim_left(int amount) const;
		[[nodiscard]] Rect2i trim_right(int amount) const;
		[[nodiscard]] Rect2i trim_top(int amount) const;
		[[nodiscard]] Rect2i trim_bottom(int amount) const;
		[[nodiscard]] Rect2i trim_to_square() const;
		[[nodiscard]] Rect2i expand(int amount) const;
		[[nodiscard]] std::optional<Rect2i> intersect(const Rect2i& other) const;

	};

	std::string to_string(const Rect2i&);
}
