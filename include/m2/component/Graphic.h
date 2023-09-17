#pragma once
#include "../Component.h"
#include "../Sprite.h"
#include "../VecI.h"
#include "../VecF.h"
#include "../m3/VecF.h"
#include "../m3/Line.h"
#include "../m3/Plane.h"
#include "../RectF.h"
#include <functional>

namespace m2 {
	/// Returns a vector from camera to given position in meters.
	/// Hint: (position - camera.position)
	VecF camera_to_position_m(const VecF& position);

	/// Returns a vector from camera to given position in pixels.
	/// For meters to pixels conversion, game_ppm is used.
	VecF camera_to_position_px(const VecF& position);

	/// Returns a vector from screen origin (top-left corner) to given position in pixels.
	/// For meter to pixels conversion, game_ppm is used.
	VecF screen_origin_to_position_px(const VecF& position);
}

namespace m3 {
	VecF camera_position_m();
	VecF player_position_m();

	/// Returns the width of the plane seen by the camera in its equator
	float visible_width_m();

	/// Returns the PPM at the plane seen by the camera in its equator.
	/// PPM should stay the same along the plane seen by the camera.
	float ppm();

	Line camera_to_position(const VecF& position); // ray from camera to position
	Plane player_to_camera();

	std::optional<VecF> player_to_projection_of_position_m(const VecF& position);
	std::optional<m2::VecF> projection_of_position_m(const VecF& position); // Centered around player
	std::optional<m2::VecF> projection_of_position_px(const VecF& position); // Centered around player
	std::optional<m2::VecF> screen_origin_to_projection_of_position_px(const VecF& position);
}

namespace m2 {
	struct Graphic : public Component {
		using Callback = std::function<void(Graphic&)>;
		Callback pre_draw{};
		Callback on_draw{};
		Callback on_effect{}; // For drawing the health bar??
		Callback post_draw{};

		const Sprite* sprite{};
		pb::SpriteEffectType draw_sprite_effect{pb::NO_SPRITE_EFFECT};
		float draw_angle{};
		float z{};
		std::optional<float> draw_addon_health_bar; /// [0,1]

		Graphic() = default;
		explicit Graphic(uint64_t object_id);
		explicit Graphic(uint64_t object_id, const Sprite& sprite);

		/// Returns a vector from the sprite's center pixel to the sprite's graphical origin.
		/// The graphical origin should align with the object's position, not the sprite's center pixel.
		[[nodiscard]] VecF sprite_center_to_sprite_origin_px() const;

		/// Returns a vector from screen origin (top-left) to the center of the sprite that should be drawn.
		/// Returns screen_origin_to_position_px(position) - sprite_center_to_sprite_origin_px() if sprite is non-NULL.
		/// Returns screen_origin_to_position_px(position) if sprite is NULL.
		[[nodiscard]] VecF screen_origin_to_sprite_center_px() const;

		static void default_draw(Graphic& gfx);
		static void default_draw_addons(Graphic& gfx);

		/// Color the world cell with the given color
		static void color_cell(const VecI& cell, SDL_Color color);
		static void color_rect(const RectF& world_coordinates_m, SDL_Color color);
		static void color_disk(const VecF& center_position_m, float radius_m, const SDL_Color& color);
		static void draw_cross(const VecF& world_position, SDL_Color color);
	};
}
