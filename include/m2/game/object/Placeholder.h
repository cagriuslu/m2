#pragma once
#include "m2/Object.h"
#include <m2/video/Sprite.h>

namespace m2::obj {
	Id CreateBackgroundPlaceholder(const VecF& pos, m2g::pb::SpriteType spriteType, BackgroundDrawLayer layer);
	Id CreateForegroundPlaceholder(const VecF& pos, float orientation, const std::optional<m2g::pb::SpriteType>& spriteType);
}
