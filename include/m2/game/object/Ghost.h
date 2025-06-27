#pragma once
#include "m2/Object.h"
#include <m2/video/Sprite.h>

namespace m2::obj {
	/// Creates an object that follows the mouse pointer around. roundToBin, if positive, applies binning to the position.
	Id CreateGhost(m2g::pb::SpriteType spriteType, int roundToBin = 1);
}
