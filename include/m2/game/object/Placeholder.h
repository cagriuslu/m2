#pragma once
#include "m2/Object.h"
#include "m2/Sprite.h"
#include "m2/math/VecF.h"

namespace m2::obj {
	Id create_background_placeholder(const VecF& pos, const Sprite& sprite, BackgroundLayer layer);
	Id create_foreground_placeholder(const VecF& pos, const Sprite& sprite);
}
