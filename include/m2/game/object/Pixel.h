#pragma once
#include "m2/Object.h"
#include "m2/math/VecF.h"

namespace m2::obj {
	Id create_pixel(const VecF& pos, SDL_Color color);
}
