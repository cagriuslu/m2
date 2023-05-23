#ifndef M2_LIGHT_H
#define M2_LIGHT_H

#include "../Component.h"
#include <functional>

namespace m2 {
	struct Light : public Component {
		uint16_t dynamic_category_bits{}; // 0: static lightning, else: dynamic lightning
		float radius_m{};
		std::function<void(Light&)> on_draw;

		Light() = default;
		explicit Light(Id object_id);

		static void default_draw(Light& lig);
	};
}

#endif //M2_LIGHT_H
