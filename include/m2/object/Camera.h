#ifndef M2_CAMERA_H
#define M2_CAMERA_H

#include <m2/Object.h>

namespace m2::obj {
    struct Camera : public ObjectImpl {
		bool draw_grid_lines{false};
        Vec2f offset;
    };

    std::pair<Object&, Id> create_camera();
}

#endif //M2_CAMERA_H
