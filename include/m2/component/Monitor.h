#ifndef M2_MONITOR_H
#define M2_MONITOR_H

#include "../Component.h"

namespace m2::component {
	struct Monitor : public Component {
		void (*prePhysics)(Monitor&);
		void (*postPhysics)(Monitor&);
		void (*preGraphics)(Monitor&);
		void (*postGraphics)(Monitor&);

		Monitor() = default;
		explicit Monitor(uint64_t object_id);
	};
}

#endif //M2_MONITOR_H
