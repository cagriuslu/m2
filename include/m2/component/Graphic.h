#ifndef M2_GRAPHIC_H
#define M2_GRAPHIC_H

#include "../Component.h"
#include <functional>

namespace m2 {
	struct Object;
}

namespace m2::comp {
	struct Graphic : public Component {
		SDL_Texture *texture{};
		SDL_Rect textureRect{};
		Vec2f center_px;
		float angle{};
		std::function<void(Graphic&)> on_draw;

		Graphic() = default;
		explicit Graphic(uint64_t object_id);

		[[nodiscard]] Object& parent() const;

		static void default_draw(Graphic& gfx);
		static void default_draw_healthbar(Graphic& gfx, float healthRatio);
	};
}

#endif //M2_GRAPHIC_H
