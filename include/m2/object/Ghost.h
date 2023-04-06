#ifndef M2_GHOST_H
#define M2_GHOST_H

#include <m2/Object.h>
#include <m2/Sprite.h>

namespace m2::obj {
	/// Creates an object that follows the mouse pointer around
	m2::Id create_ghost(const Sprite& sprite);
}

#endif //M2_GHOST_H
