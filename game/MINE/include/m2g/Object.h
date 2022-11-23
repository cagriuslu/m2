#ifndef MINE_OBJECT_H
#define MINE_OBJECT_H

#include <m2/Object.h>
#include <m2/Value.h>
#include <functional>

namespace m2g {
    void post_tile_create(m2::Object& obj, pb::SpriteType sprite_type);
	m2::VoidValue fg_object_loader(m2::Object& obj, pb::ObjectType object_type);
}

#endif //MINE_OBJECT_H
