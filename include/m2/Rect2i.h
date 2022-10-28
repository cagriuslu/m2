#ifndef M2_RECT2I_H
#define M2_RECT2I_H

#include "Vec2i.h"
#include <SDL2/SDL_rect.h>
#include <string>

namespace m2 {
	struct Rect2i {
		int x, y, w, h;

		Rect2i();
		Rect2i(int x, int y, int w, int h);
		explicit Rect2i(const SDL_Rect& r);

		bool operator==(const Rect2i& other) const;
		explicit operator bool() const;
		explicit operator SDL_Rect() const;
		[[nodiscard]] bool point_in_rect(const Vec2i&) const;

		[[nodiscard]] Rect2i trim(int amount) const;
		[[nodiscard]] Rect2i trim_left(int amount) const;
		[[nodiscard]] Rect2i trim_right(int amount) const;
		[[nodiscard]] Rect2i trim_top(int amount) const;
		[[nodiscard]] Rect2i trim_bottom(int amount) const;
		[[nodiscard]] Rect2i trim_to_square() const;
		[[nodiscard]] Rect2i expand(int amount) const;

	};

	std::string to_string(const Rect2i&);
}

#endif //M2_RECT2I_H
