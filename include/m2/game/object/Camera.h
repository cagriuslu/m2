#pragma once
#include "m2/Object.h"

namespace m2::obj {
    struct Camera : public HeapObjectImpl {
        Camera() = default;
        VecF offset;
    };

    Id CreateCamera();
}
