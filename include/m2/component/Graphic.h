#pragma once
#include "../Component.h"
#include <m2/video/Sprite.h>
#include "../math/VecF.h"
#include "../m3/VecF.h"
#include "../m3/Line.h"
#include "../m3/Plane.h"
#include "../math/RectF.h"
#include <Level.pb.h>
#include <functional>
#include <m2/video/Color.h>

namespace m2 {
	bool is_projection_type_parallel(pb::ProjectionType pt);
	bool is_projection_type_perspective(pb::ProjectionType pt);

	/// Returns a vector from camera to given position in meters.
	/// Hint: (position - camera.position)
	VecF camera_to_position_m(const VecF& position);

	/// Returns a vector from camera to given position in pixels.
	/// For meters to pixels conversion, game_ppm is used.
	VecF camera_to_position_dstpx(const VecF& position);

	/// Returns a vector from screen origin (top-left corner) to given position in pixels.
	/// For meter to pixels conversion, game_ppm is used.
	VecF screen_origin_to_position_dstpx(const VecF& position);

	/// Returns a vector from screen origin (top-left) to the center of the sprite that should be drawn.
	/// Returns screen_origin_to_position_dstpx(position) - sprite.center_to_origin_dstpx() if sprite is non-NULL.
	VecF screen_origin_to_sprite_center_dstpx(const VecF& position, const Sprite& sprite, SpriteVariant sprite_variant);
}

namespace m3 {
	/// Returns the position of the camera in 3D.
	VecF camera_position_m();

	/// Returns the position of the focus in 3D.
	/// Focus point will be placed in the middle of the screen.
	VecF focus_position_m();

	/// Returns the width of the plane seen by the camera in its equator
	float visible_width_m();

	/// Returns the PPM at the plane seen by the camera in its equator.
	float ppm();

	/// Returns the line from camera to given position.
	Line camera_to_position_line(const VecF& position);

	/// Returns the plane that faces the camera from the focus point.
	/// This plane is called the "camera plane"
	Plane focus_to_camera_plane();

	/// Returns the projection of a given position on the camera plane.
	std::optional<VecF> projection(const VecF& position);

	/// Returns a 3D vector from the focus point to the projection of a given position.
	/// Since the focus point is at the center of the camera plane, this vector must also lie on the plane.
	std::optional<VecF> focus_to_projection_m(const VecF& position);

	/// Returns a vector from the focus point to the projection of a given position, but using the camera plane as the
	/// coordinate system where the focus point sits in the center. Returned vector is 2D, because both the focus point
	/// and the projection lie on the same plane (camera plane)
	std::optional<m2::VecF> focus_to_projection_in_camera_plane_coordinates_m(const VecF& position);

	/// Returns a vector from the focus point to the projection of a given position, but using the camera plane as the
	/// coordinate system where the focus point sits in the center, in destination pixels.
	std::optional<m2::VecF> focus_to_projection_in_camera_plane_coordinates_dstpx(const VecF& position);

	/// Returns a vector from the screen origins, to the projection of a given position, but using the camera plane as
	/// the coordinate system where the focus point sits in the center, in destination pixels.
	std::optional<m2::VecF> screen_origin_to_projection_along_camera_plane_dstpx(const VecF& position);
}

namespace m2 {
	struct Graphic : public Component {
		using Callback = std::function<void(Graphic&)>;

		bool enabled{true};
		bool draw{true};

		Callback pre_draw{};
		Callback on_draw{};
		Callback on_addon{};
		Callback post_draw{};

		const Sprite* sprite{};
		// If any of the entries exist, Sprite's default_variant_draw_order is overridden. First variant is drawn first.
		std::array<std::optional<SpriteVariant>, 2> variant_draw_order{};
		float draw_angle{}; // Rads
		float z{};
		std::optional<float> draw_addon_health_bar; /// [0,1]

		Graphic() = default;
		explicit Graphic(uint64_t object_id);
		explicit Graphic(uint64_t object_id, const Sprite& sprite);

		static void default_draw(const Graphic& gfx);
		static void default_draw_addons(const Graphic& gfx);

		/// Color the world cell with the given color
		static void color_cell(const VecI& cell, SDL_Color color);
		static void color_rect(const RectF& world_coordinates_m, SDL_Color color);
		static void color_rect(const RectF& world_coordinates_m, const RGB& color);
		static void color_disk(const VecF& center_position_m, float radius_m, const SDL_Color& color);
		static void draw_cross(const VecF& world_position, SDL_Color color);
		static void draw_line(const VecF& world_position_1, const VecF& world_position_2, SDL_Color color);
		static void draw_vertical_line(float x, SDL_Color color);
		static void draw_horizontal_line(float y, SDL_Color color);

		// Global Modifiers

		/// If dimming_with_exceptions is enabled, and the given object_id is not in the exceptions, dim the given texture.
		static bool dim_rendering_if_necessary(Id object_id, SDL_Texture* texture);
		/// Undim the given texture
		static void undim_rendering(SDL_Texture* texture);
	};

	void draw_real_2d(const VecF& position, const Sprite& sprite, SpriteVariant sprite_variant, float angle);
	void draw_fake_3d(const VecF& position, const Sprite& sprite, SpriteVariant sprite_variant, float angle, bool is_foreground, float z);
}
