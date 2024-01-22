#pragma once
#include "m2/Object.h"

namespace m2::obj {
    struct Camera : public ObjectImpl {
        VecF offset;
    };

    Id create_camera();
}
