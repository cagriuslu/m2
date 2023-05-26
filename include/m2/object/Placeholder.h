#pragma once
#include <m2/Object.h>
#include <m2/Sprite.h>
#include <m2/Vec2f.h>

namespace m2::obj {
	Id create_placeholder(const Vec2f& pos, const Sprite& sprite, bool is_fg);
}
