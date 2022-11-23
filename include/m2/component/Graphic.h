#ifndef M2_COMPONENT_GRAPHIC_H
#define M2_COMPONENT_GRAPHIC_H

#include "../Component.h"
#include "../Sprite.h"
#include "../Vec2i.h"
#include "../Vec2f.h"
#include <functional>

namespace m2 {
	/// Returns a vector from camera to given position in meters.
	/// Hint: (position - camera.position)
	Vec2f camera_to_position_m(const Vec2f& position);

	/// Returns a vector from camera to given position in pixels.
	/// For meters to pixels conversion, game_ppm is used.
	Vec2f camera_to_position_px(const Vec2f& position);

	/// Returns a vector from screen origin (top-left corner) to given position in pixels.
	/// For meter to pixels conversion, game_ppm is used.
	Vec2f screen_origin_to_position_px(const Vec2f& position);
}

namespace m2 {
	struct Graphic : public Component {
		const Sprite* sprite{};
		pb::SpriteEffectType draw_sprite_effect{pb::NO_SPRITE_EFFECT};
		float draw_angle{};
		std::function<void(Graphic&)> on_draw{};

		Graphic() = default;
		explicit Graphic(uint64_t object_id);
		explicit Graphic(uint64_t object_id, const Sprite& sprite);

		/// Returns a vector from the sprite's center pixel to the sprite's graphical origin.
		/// The graphical origin should align with the object's position, not the sprite's center pixel.
		[[nodiscard]] Vec2f sprite_center_to_sprite_origin_px() const;

		/// Returns a vector from screen origin to the center of the sprite that should be drawn.
		/// Returns screen_origin_to_position_px(position) - sprite_center_to_sprite_origin_px().
		[[nodiscard]] Vec2f screen_origin_to_sprite_center_px() const;

		static void default_draw(Graphic& gfx);
		static void default_draw_healthbar(Graphic& gfx, float healthRatio);
	};
}

#endif //M2_COMPONENT_GRAPHIC_H
