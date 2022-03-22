#ifndef M2_CAMERA_H
#define M2_CAMERA_H

#include <m2/object.hh>

namespace m2::object::camera {
    struct Data : public object::Data {
        vec2f offset;
    };

    std::pair<Object&, ID> create();
}

#endif //M2_CAMERA_H
