#ifndef M2_PIXEL_H
#define M2_PIXEL_H

#include <m2/Object.h>
#include <m2/Vec2f.h>

namespace m2::obj {
	Id create_pixel(const Vec2f& pos, SDL_Color color);
}

#endif //M2_PIXEL_H
