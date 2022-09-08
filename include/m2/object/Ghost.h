#ifndef M2_GHOST_H
#define M2_GHOST_H

#include <m2/Object.h>
#include <m2/Sprite.h>

namespace m2::obj {
	std::pair<m2::Object&, m2::ID> create_ghost(const Sprite& sprite);
}

#endif //M2_GHOST_H
