#pragma once
#include "m2/Object.h"
#include <m2/video/Sprite.h>

namespace m2::obj {
	/// Creates an object that follows the mouse pointer around
	Id create_ghost(const Sprite& sprite);
}
