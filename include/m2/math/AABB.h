#pragma once
#include "../math/VecF.h"
#include "../math/VecI.h"
#include <box2d/b2_collision.h>

namespace m2 {
	/// Axis-aligned bounding box
	struct AABB {
		VecF top_left;
		VecF bottom_right;

		AABB();
		AABB(const VecF& top_left, const VecF& bottom_right);
		AABB(const VecI& top_left, const VecI& bottom_right);
        AABB(const VecF& center, float side);
        AABB(const VecF& center, float width, float height);

		[[nodiscard]] float GetWidth() const;
		[[nodiscard]] float GetHeight() const;
		[[nodiscard]] VecF GetCenterPoint() const;
		explicit operator bool() const;
        explicit operator b2AABB() const;
	};
}
