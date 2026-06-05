#pragma once
#include "m2/Object.h"
#include <m2/video/Sprite.h>

namespace m2::obj {
	Pool<Object>::Iterator CreateTile(m2g::pb::FlatGraphicsLayer layer, const VecF& position, m2g::pb::SpriteType spriteType);
}
