#pragma once
#include "m2/Object.h"

namespace m2::obj {
	Id create_texture_patch(const VecF& pos, SDL_Texture* texture, RectI rect, int ppm);
}
