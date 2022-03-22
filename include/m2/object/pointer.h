#ifndef M2_POINTER_H
#define M2_POINTER_H

#include <m2/object.hh>

namespace m2::object::pointer {
    std::pair<Object&, ID> create();
}

#endif //M2_POINTER_H
