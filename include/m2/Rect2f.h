#ifndef M2_RECT2F_H
#define M2_RECT2F_H

#include <SDL2/SDL_rect.h>
#include <Rect2i.pb.h>
#include "Vec2f.h"
#include <optional>

namespace m2 {
	struct Rect2i;

	struct Rect2f {
		float x, y, w, h;

		Rect2f();
		Rect2f(float x, float y, float w, float h);
		Rect2f(const Vec2f& offset_from_origin, const Vec2f& dims);
		explicit Rect2f(const Rect2i& r);
		explicit Rect2f(const SDL_FRect& r);
		explicit Rect2f(const SDL_Rect& r);
		explicit Rect2f(const pb::Rect2i& r);

		explicit operator bool() const;
		explicit operator SDL_FRect() const;
		explicit operator SDL_Rect() const;

		[[nodiscard]] float area() const;
		[[nodiscard]] Rect2f shift_origin(const Vec2f& direction) const;
		[[nodiscard]] std::optional<Rect2f> intersect(const Rect2f& other) const;
	};
}

#endif //M2_RECT2F_H
