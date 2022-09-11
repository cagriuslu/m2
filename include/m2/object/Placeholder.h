#ifndef M2_PLACEHOLDER_H
#define M2_PLACEHOLDER_H

#include <m2/Object.h>
#include <m2/Sprite.h>
#include <m2/Vec2f.h>

namespace m2::obj {
	ID create_placeholder(const Vec2f& pos, const Sprite& sprite, bool is_fg);
}

#endif //M2_PLACEHOLDER_H
