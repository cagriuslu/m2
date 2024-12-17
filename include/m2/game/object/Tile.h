#pragma once
#include "m2/Object.h"
#include <m2/video/Sprite.h>

namespace m2::obj {
	Pool<Object>::Iterator create_tile(BackgroundLayer layer, const VecF& position, const m2::Sprite& sprite);
	Pool<Object>::Iterator create_tile_foreground_companion(const VecF& position, const m2::Sprite& sprite);
}
