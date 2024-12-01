#pragma once
#include "../math/VecF.h"
#include "../math/VecI.h"
#include <box2d/b2_collision.h>

namespace m2 {
	/// Axis-aligned bounding box
	struct Aabb {
		VecF top_left;
		VecF bottom_right;

		Aabb();
		Aabb(const VecF& top_left, const VecF& bottom_right);
		Aabb(const VecI& top_left, const VecI& bottom_right);
        Aabb(const VecF& center, float side);
        Aabb(const VecF& center, float width, float height);

		[[nodiscard]] float width() const;
		[[nodiscard]] float height() const;
		[[nodiscard]] VecF center() const;
		explicit operator bool() const;
        explicit operator b2AABB() const;
	};
}
