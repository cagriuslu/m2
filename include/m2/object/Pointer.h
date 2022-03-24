#ifndef M2_POINTER_H
#define M2_POINTER_H

#include <m2/Object.h>

namespace m2::object {
    std::pair<Object&, ID> create_pointer();
}

#endif //M2_POINTER_H
