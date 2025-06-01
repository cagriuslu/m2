#pragma once
#include "m2/Object.h"
#include <m2/video/Sprite.h>

namespace m2::obj {
	Id create_background_placeholder(const VecF& pos, m2g::pb::SpriteType spriteType, BackgroundDrawLayer layer);
	Id create_foreground_placeholder(const VecF& pos, float orientation, const std::optional<m2g::pb::SpriteType>& spriteType);
}
