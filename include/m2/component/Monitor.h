#ifndef M2_MONITOR_H
#define M2_MONITOR_H

#include "../Component.h"
#include <functional>

namespace m2::component {
	struct Monitor : public Component {
		std::function<void(Monitor&)> pre_phy;
		std::function<void(Monitor&)> post_phy;
		std::function<void(Monitor&)> pre_gfx;
		std::function<void(Monitor&)> post_gfx;

		Monitor() = default;
		explicit Monitor(uint64_t object_id);
	};
}

#endif //M2_MONITOR_H
