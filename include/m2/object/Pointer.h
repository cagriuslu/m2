#ifndef M2_POINTER_H
#define M2_POINTER_H

#include <m2/Object.h>

namespace m2::obj {
    std::pair<Object&, Id> create_pointer();
}

#endif //M2_POINTER_H
