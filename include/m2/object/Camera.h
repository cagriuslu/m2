#ifndef M2_CAMERA_H
#define M2_CAMERA_H

#include <m2/Object.h>

namespace m2::object {
    struct Camera : public ObjectImpl {
        vec2f offset;
    };

    std::pair<Object&, ID> create_camera();
}

#endif //M2_CAMERA_H
