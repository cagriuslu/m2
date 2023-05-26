#pragma once
#include <m2/Object.h>
#include <m2/Sprite.h>

namespace m2::obj {
	std::pair<Object&, Id> create_tile(const Vec2f& position, const m2::Sprite& sprite);
	std::pair<Object&, Id> create_tile_foreground_companion(const Vec2f& position, const m2::Sprite& sprite);
}
