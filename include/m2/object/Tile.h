#pragma once
#include <m2/Object.h>
#include <m2/Sprite.h>

namespace m2::obj {
	std::pair<Object&, Id> create_tile(BackgroundLayer layer, const VecF& position, const m2::Sprite& sprite);
	std::pair<Object&, Id> create_tile_foreground_companion(const VecF& position, const m2::Sprite& sprite);
}
