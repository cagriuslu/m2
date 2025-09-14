#pragma once
#include <m2/math/VecF.h>
#include "../Component.h"
#include <functional>

namespace m2 {
	struct Light : public Component {
		VecF position;
		uint16_t dynamicCategoryBits{}; // 0: static lightning, else: dynamic lightning
		float radiusM{};
		std::function<void(Light&)> onDraw;

		Light() = default;
		explicit Light(Id object_id);

		static void DefaultDrawCallback(Light& lig);
	};
}
