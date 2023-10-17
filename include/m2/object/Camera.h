#pragma once
#include <m2/Object.h>

namespace m2::obj {
    struct Camera : public ObjectImpl {
		bool draw_grid_lines{false};
        VecF offset;
    };

    Id create_camera();
}
