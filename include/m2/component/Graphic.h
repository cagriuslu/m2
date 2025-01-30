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
	VecF CameraToPositionVecM(const VecF& position);
	/// Returns a vector from camera to given position in output pixels.
	/// For meters to pixels conversion, Game Output PPM is used.
	VecF CameraToPositionVecPx(const VecF& position);
	/// Returns a vector from screen origin (top-left corner) to given position in output pixels.
	/// For meters to pixels conversion, Game Output PPM is used.
	VecF ScreenOriginToPositionVecPx(const VecF& position);
	/// Returns the corresponding position of a pixel in 2D world coordiantes. Only applicable to parallel projection.
	VecF PixelToPositionVecM(const VecI& pixelPosition);
	/// Returns the boundaries of the Game viewport in 2D world coordinates. Only applicable to parallel projection.
	RectF ViewportM();
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
	struct Graphic final : Component {
		bool enabled{true};
		bool draw{true};

		using Callback = std::function<void(Graphic&)>;
		Callback pre_draw{};
		Callback on_draw{};
		Callback post_draw{};

		std::variant<std::monostate, const Sprite*, const pb::TextLabel*> visual;

		/// Only applicable to sprites. If any of the entries exist, Sprite's default_variant_draw_order is overridden.
		/// First variant is drawn first.
		std::array<std::optional<SpriteVariant>, 2> variant_draw_order{};

		/// Only applicable to text labels.
		RectI textLabelRect{};

		float z{};

		Graphic() = default;
		explicit Graphic(uint64_t object_id); // TODO can this be deleted
		explicit Graphic(uint64_t object_id, const std::variant<Sprite, pb::TextLabel>&);

		/// Default draw callback that's capable of drawing sprites, foreground components, sprite effects, and text
		/// labels.
		static void DefaultDrawCallback(Graphic& gfx);

		/// Color the world cell with the given color
		static void color_cell(const VecI& cell, SDL_Color color);
		static void color_rect(const RectF& world_coordinates_m, SDL_Color color);
		static void color_rect(const RectF& world_coordinates_m, const RGB& color);
		static void color_rect(const RectF& world_coordinates_m, const RGBA& color);
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

	/// Draws the given texture to the 2D World. Before applying extra rotation to the texture, the rotation of the
	/// source texture is corrected in case it has a rotation of its own.
	void DrawTextureIn2dWorld(SDL_Renderer* renderer, SDL_Texture* sourceTexture, const SDL_Rect* sourceRect,
			float originalRotationOfSourceTextureInRadians, float outputToSourcePpmRatio,
			const VecF& textureCenterToTextureOriginVecInOutputPixels,
			const VecF& screenOriginToTextureCenterVecInOutputPixels, float rotationToApplyInRadians);
	/// Draws the given texture to the 3D World. Before applying extra rotation to the texture, the rotation of the
	/// source texture is corrected in case it has a rotation of its own.
	void DrawTextureIn3dWorld(SDL_Renderer* renderer, SDL_Texture* sourceTexture, const SDL_Rect* sourceRect,
			float sourcePpm, const VecF& sourceCenterToOriginVectorInOutputPixels,
			float originalRotationOfSourceTextureInRadians, const VecF& sourceTextureSheetDimensions,
			const VecF& xyPositionInWorldM, float zPositionInWorldM, float rotationToApplyInRadians, bool isForeground);
}
