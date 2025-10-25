#pragma once
#include "m2/Object.h"

namespace m2::obj {
    struct Camera : public ObjectImpl {
        explicit Camera(Object& object) : ObjectImpl(object) {}
        VecF offset;
    };

    Id CreateCamera();
}
