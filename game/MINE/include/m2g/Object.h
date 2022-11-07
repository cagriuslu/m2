#ifndef MINE_OBJECT_H
#define MINE_OBJECT_H

#include <m2/Object.h>
#include <m2/Value.h>

namespace m2g {
	m2::VoidValue fg_object_loader(m2::Object& obj, pb::ObjectType object_type);
}

#endif //MINE_OBJECT_H
