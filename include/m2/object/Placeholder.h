#pragma once
#include <m2/Object.h>
#include <m2/Sprite.h>
#include <m2/VecF.h>

namespace m2::obj {
	Id create_placeholder(const VecF& pos, const Sprite& sprite, bool is_fg);
}
