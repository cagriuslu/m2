#ifndef M2_MONITOR_H
#define M2_MONITOR_H

#include "../Component.h"
#include <functional>

namespace m2 {
	struct Object;
}

namespace m2::comp {
	struct Monitor : public Component {
		using Callback = std::function<void(Monitor&)>;
		Callback pre_phy;
		Callback post_phy;
		Callback pre_gfx;
		Callback post_gfx;

		Monitor() = default;
		explicit Monitor(uint64_t object_id);
		explicit Monitor(uint64_t object_id, const Callback& pre_phy, const Callback& post_phy, const Callback& pre_gfx, const Callback& post_gfx);

		[[nodiscard]] Object& parent() const;
	};
}

#endif //M2_MONITOR_H
