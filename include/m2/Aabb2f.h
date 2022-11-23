#ifndef M2_AABB2F_H
#define M2_AABB2F_H

#include "Vec2f.h"
#include "Vec2i.h"
#include <box2d/b2_collision.h>

namespace m2 {
	struct Aabb2f {
		Vec2f top_left;
		Vec2f bottom_right;

		Aabb2f();
		Aabb2f(const Vec2f& top_left, const Vec2f& bottom_right);
		Aabb2f(const Vec2i& top_left, const Vec2i& bottom_right);
        Aabb2f(const Vec2f& center, float side);
        Aabb2f(const Vec2f& center, float width, float height);

		[[nodiscard]] float width() const;
		[[nodiscard]] float height() const;
		[[nodiscard]] Vec2f center() const;
		explicit operator bool() const;
        explicit operator b2AABB() const;
	};
}

#endif //M2_AABB2F_H
