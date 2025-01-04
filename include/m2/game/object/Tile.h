#pragma once
#include "m2/Object.h"
#include <m2/video/Sprite.h>

namespace m2::obj {
	Pool<Object>::Iterator create_tile(BackgroundLayer layer, const VecF& position, m2g::pb::SpriteType spriteType);
}
