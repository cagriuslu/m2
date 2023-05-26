#pragma once
#include <m2/Object.h>

namespace m2::obj {
    struct Camera : public ObjectImpl {
		bool draw_grid_lines{false};
        Vec2f offset;
    };

    std::pair<Object&, Id> create_camera();
}
