#ifndef M2_COMPONENT_GRAPHIC_H
#define M2_COMPONENT_GRAPHIC_H

#include "../Component.h"
#include "../Sprite.h"
#include "../Vec2i.h"
#include "../Vec2f.h"
#include <functional>

namespace m2 {
	struct Object;

	/// Returns (position - camera.position)
	Vec2f offset_from_camera_m(Vec2f position);

	/// Returns (position - camera.position) * game_ppm
	Vec2i offset_from_camera_px(Vec2f position);
}

namespace m2::comp {
	struct Graphic : public Component {
		const Sprite* sprite{};
		pb::SpriteEffectType effect_type{pb::NO_SPRITE_EFFECT};
		float angle{};
		std::function<void(Graphic&)> on_draw{};

		Graphic() = default;
		explicit Graphic(uint64_t object_id);
		explicit Graphic(uint64_t object_id, const Sprite& sprite);

		[[nodiscard]] Object& parent() const;
		[[nodiscard]] Vec2i offset_from_screen_center_px() const;
		[[nodiscard]] Vec2i offset_from_screen_origin_px() const;

		static void default_draw(Graphic& gfx);
		static void default_draw_healthbar(Graphic& gfx, float healthRatio);
	};
}

#endif //M2_COMPONENT_GRAPHIC_H
