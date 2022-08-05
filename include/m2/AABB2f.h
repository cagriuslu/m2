#ifndef M2_AABB2F_H
#define M2_AABB2F_H

#include "Vec2f.h"
#include "Vec2i.h"

namespace m2 {
	struct AABB2f {
		Vec2f top_left;
		Vec2f bottom_right;

		AABB2f();
		AABB2f(const Vec2f& top_left, const Vec2f& bottom_right);
		AABB2f(const Vec2i& top_left, const Vec2i& bottom_right);

		[[nodiscard]] float width() const;
		[[nodiscard]] float height() const;
		[[nodiscard]] Vec2f center() const;
		explicit operator bool() const;
	};
}

#endif //M2_AABB2F_H
