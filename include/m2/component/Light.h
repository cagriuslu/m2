#pragma once
#include "../Component.h"
#include <functional>

namespace m2 {
	struct Light : public Component {
		uint16_t dynamicCategoryBits{}; // 0: static lightning, else: dynamic lightning
		float radiusM{};
		std::function<void(Light&)> onDraw;

		Light() = default;
		explicit Light(Id object_id);

		static void DefaultDrawCallback(Light& lig);
	};
}
