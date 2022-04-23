#ifndef M2_LIGHT_H
#define M2_LIGHT_H

#include "../Component.h"
#include <functional>

namespace m2 {
	struct Object;
}

namespace m2::comp {
	struct Light : public Component {
		float radius_m{};
		std::function<void(Light&)> on_draw;

		Light() = default;
		explicit Light(ID object_id);

		[[nodiscard]] Object& parent() const;

		static void default_draw(Light& lig);
	};
}

#endif //M2_LIGHT_H
