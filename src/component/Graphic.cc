#include <m2/m3/VecF.h>
#include <m2/Component.h>
#include <m2/Game.h>
#include "m2/component/Graphic.h"
#include <m2/Object.h>

namespace {
	float camera_sin() {
		// Do not recalculate unless the camera offset changed
		static m3::VecF prev_camera_offset;
		static float sin{};

		if (const auto camera_offset = M2_LEVEL.camera_offset(); prev_camera_offset != camera_offset) {
			prev_camera_offset = camera_offset;

			sin = camera_offset.z / camera_offset.length();
		}
		return sin;
	}
	float camera_cos() {
		// Do not recalculate unless the camera offset changed
		static m3::VecF prev_camera_offset;
		static float cos{};

		if (const auto camera_offset = M2_LEVEL.camera_offset(); prev_camera_offset != camera_offset) {
			prev_camera_offset = camera_offset;

			cos = sqrtf(1.0f - camera_sin() * camera_sin());
		}
		return cos;
	}
}

bool m2::is_projection_type_parallel(const pb::ProjectionType pt) {
	return pt == pb::PARALLEL || pt == pb::PARALLEL_ISOMETRIC;
}
bool m2::is_projection_type_perspective(const pb::ProjectionType pt) {
	return pt == pb::PERSPECTIVE_YZ || pt == pb::PERSPECTIVE_XYZ;
}

m2::VecF m2::CameraToPositionVecM(const VecF& position) {
	const auto* camera = M2_LEVEL.objects.get(M2_LEVEL.camera_id);
	return position - camera->position;
}
m2::VecF m2::CameraToPositionVecPx(const VecF& position) {
	// Find the vector from camera to position and multiply with output PPM to convert into output pixels
	return CameraToPositionVecM(position) * M2_GAME.Dimensions().OutputPixelsPerMeter();
}
m2::VecF m2::ScreenOriginToPositionVecPx(const VecF& position) {
	return CameraToPositionVecPx(position) + VecF{M2_GAME.Dimensions().WindowDimensions().x / 2, M2_GAME.Dimensions().WindowDimensions().y / 2 };
}

m3::VecF m3::camera_position_m() {
	const auto* camera = M2_LEVEL.objects.get(M2_LEVEL.camera_id);
	const auto raw_camera_position = VecF{camera->position.x, camera->position.y, 0.0f};
	const auto camera_position = raw_camera_position + M2_LEVEL.camera_offset();
	return camera_position;
}
m3::VecF m3::focus_position_m() {
	return {M2_PLAYER.position.x, M2_PLAYER.position.y, M2G_PROXY.focus_point_height};
}
float m3::visible_width_m() {
	// Do not recalculate unless the distance or FOV has changed
	static m3::VecF prev_camera_offset;
	static auto prev_horizontal_fov = INFINITY;

	static float visible_width{};
	if (const auto camera_offset = M2_LEVEL.camera_offset(); prev_camera_offset != camera_offset || prev_horizontal_fov != M2_LEVEL.horizontal_fov()) {
		prev_camera_offset = camera_offset;
		prev_horizontal_fov = M2_LEVEL.horizontal_fov();

		const auto tan_of_half_horizontal_fov = tanf(m2::to_radians(M2_LEVEL.horizontal_fov()) / 2.0f);
		visible_width = 2 * camera_offset.length() * tan_of_half_horizontal_fov;
	}
	return visible_width;
}
float m3::ppm() {
	return static_cast<float>(M2_GAME.Dimensions().Game().w) / visible_width_m();
}

m3::Line m3::camera_to_position_line(const VecF& position) {
	return Line::from_points(camera_position_m(), position);
}
m3::Plane m3::focus_to_camera_plane() {
	const auto focus_position = focus_position_m();
	const auto normal = camera_position_m() - focus_position;
	return Plane{normal, focus_position};
}
std::optional<m3::VecF> m3::projection(const VecF& position) {
	auto [intersection_point, forward_intersection] = focus_to_camera_plane()
			.intersection(camera_to_position_line(position));
	if (not forward_intersection) {
		return {};
	}
	return intersection_point;
}
std::optional<m3::VecF> m3::focus_to_projection_m(const VecF& position) {
	const auto proj = projection(position);
	if (not proj) {
		return std::nullopt;
	}
	return *proj - focus_position_m();
}
std::optional<m2::VecF> m3::focus_to_projection_in_camera_plane_coordinates_m(const VecF& position) {
	const auto focus_to_projection = focus_to_projection_m(position);
	if (not focus_to_projection) {
		return {};
	}

	float horizontal_projection, vertical_projection;
	if (M2_LEVEL.ProjectionType() == m2::pb::PERSPECTIVE_YZ) {
		static const auto unit_vector_along_x = VecF{1.0f, 0.0f, 0.0f};
		horizontal_projection = focus_to_projection->dot(unit_vector_along_x);
		const auto projection_x = VecF{horizontal_projection, 0.0f, 0.0f};

		// y-axis of the projection plane is not along the real y-axis
		// Use dot product, or better, find the length and the sign of it
		const auto projection_y = *focus_to_projection - projection_x;
		const auto projection_y_length = projection_y.length();
		const auto projection_y_sign = 0 <= projection_y.y ? 1.0f : -1.0f;
		vertical_projection = projection_y_length * projection_y_sign;
	} else if (M2_LEVEL.ProjectionType() == m2::pb::PERSPECTIVE_XYZ) {
		static const auto horizontal_unit_vector = VecF{m2::SQROOT_2, -m2::SQROOT_2, 0.0f};
		horizontal_projection = focus_to_projection->dot(horizontal_unit_vector);

		const auto vertical_unit_vector = VecF{camera_sin() / m2::SQROOT_2, camera_sin() / m2::SQROOT_2, -camera_cos()};
		vertical_projection = focus_to_projection->dot(vertical_unit_vector);
	} else {
		throw M2_ERROR("Invalid ProjectionType");
	}
	return m2::VecF{horizontal_projection, vertical_projection};
}
std::optional<m2::VecF> m3::focus_to_projection_in_camera_plane_coordinates_dstpx(const VecF& position) {
	const auto focus_to_projection_along_camera_plane = focus_to_projection_in_camera_plane_coordinates_m(position);
	if (not focus_to_projection_along_camera_plane) {
		return {};
	}
	const auto pixels_per_meter = ppm();
	return *focus_to_projection_along_camera_plane * pixels_per_meter;
}
std::optional<m2::VecF> m3::screen_origin_to_projection_along_camera_plane_dstpx(const VecF& position) {
	const auto focus_to_projection_along_camera_plane_px = focus_to_projection_in_camera_plane_coordinates_dstpx(position);
	if (not focus_to_projection_along_camera_plane_px) {
		return {};
	}
	return *focus_to_projection_along_camera_plane_px + m2::VecF{M2_GAME.Dimensions().WindowDimensions().x / 2, M2_GAME.Dimensions().WindowDimensions().y / 2 };
}

m2::Graphic::Graphic(const Id object_id) : Component(object_id) {}
m2::Graphic::Graphic(const uint64_t object_id, const std::variant<Sprite, pb::TextLabel>& spriteOrTextLabel)
		: Component(object_id), on_draw(DefaultDrawCallback), on_addon(default_draw_addons) {
	if (std::holds_alternative<Sprite>(spriteOrTextLabel)) {
		visual = &std::get<Sprite>(spriteOrTextLabel);
	} else {
		visual = &std::get<pb::TextLabel>(spriteOrTextLabel);
	}
}

void m2::Graphic::DefaultDrawCallback(Graphic& gfx) {
	if (std::holds_alternative<const Sprite*>(gfx.visual)) {
		const auto& sprite = *std::get<const Sprite*>(gfx.visual);

		// Check if foreground or background
		const bool is_foreground = M2_LEVEL.graphics.get_id(&gfx);
		const auto projector = is_projection_type_perspective(M2_LEVEL.ProjectionType())
				? &Sprite::DrawIn3dWorld
				: &Sprite::DrawIn2dWorld;
		const auto spriteDrawer = [&](const SpriteVariant sv) -> void {
			(sprite.*projector)(gfx.owner().position, sv, gfx.draw_angle, is_foreground, gfx.z);
		};

		// Dim the sprite if dimming mode is enabled. TODO Dimming is implemented only for default variant.
		const bool dimmed = dim_rendering_if_necessary(gfx.owner_id(), sprite.Texture(SpriteVariant{}));

		// First, try to draw something using the variant_draw_order, which overrides default_variant_draw_order.
		bool isAnythingDrawn = false;
		for (auto optionalSpriteVariant : gfx.variant_draw_order) {
			if (optionalSpriteVariant) {
				spriteDrawer(*optionalSpriteVariant);
				isAnythingDrawn = true;
			}
		}
		// If nothing is drawn, fallback to default variant draw order of the sprite.
		if (not isAnythingDrawn) {
			// If the default variant draw order list is empty, draw only the default variant.
			if (sprite.DefaultVariantDrawOrder().empty()) {
				spriteDrawer(DefaultVariant{});
			} else {
				for (const auto& sprite_variant : sprite.DefaultVariantDrawOrder()) {
					spriteDrawer(
							sprite_variant == pb::SpriteEffectType::__NO_SPRITE_EFFECT
							? SpriteVariant{DefaultVariant{}}
							: sprite_variant);
				}
			}
		}

		// If dimming was active, we need to un-dim.
		if (dimmed) {
			undim_rendering(sprite.Texture(SpriteVariant{}));
		}
	} else if (std::holds_alternative<const pb::TextLabel*>(gfx.visual)) {
		const auto& textLabel = *std::get<const pb::TextLabel*>(gfx.visual);

		// Generate the text label if necessary
		if (not gfx.textLabelRect) {
			// Font size is the same as text height in output pixels
			gfx.textLabelRect = M2_GAME.TextLabelCache().Create(textLabel.text(), FontSizeOfTextLabel(textLabel));
		}

		// Dim the sprite if dimming mode is enabled.
		const bool dimmed = dim_rendering_if_necessary(gfx.owner_id(), M2_GAME.TextLabelCache().Texture());

		// Draw
		const bool is_foreground = M2_LEVEL.graphics.get_id(&gfx);
		const auto projector = is_projection_type_perspective(M2_LEVEL.ProjectionType())
				? &DrawTextLabelIn3dWorld
				: &DrawTextLabelIn2dWorld;
		projector(textLabel, gfx.textLabelRect, gfx.owner().position, gfx.draw_angle, is_foreground, gfx.z);

		// If dimming was active, we need to un-dim.
		if (dimmed) {
			undim_rendering(M2_GAME.TextLabelCache().Texture());
		}
	} else {
		// This function only draws visuals
	}
}

void m2::Graphic::default_draw_addons(const Graphic& gfx) {
	if (std::holds_alternative<const Sprite*>(gfx.visual)) {
		const auto& sprite = *std::get<const Sprite*>(gfx.visual);

		if (not gfx.draw_addon_health_bar) {
			return;
		}

		SDL_Rect dst_rect{};

		if (is_projection_type_parallel(M2_LEVEL.ProjectionType())) {
			const auto src_rect = static_cast<SDL_Rect>(sprite.Rect());
			const auto screen_origin_to_sprite_center_px_vec = sprite.ScreenOriginToCenterVecOutpx(gfx.owner().position, SpriteVariant{});
			dst_rect = SDL_Rect{
				iround(screen_origin_to_sprite_center_px_vec.x - (F(src_rect.w) * M2_GAME.Dimensions().OutputPixelsPerMeter() / F(sprite.Ppm()) / 2.0f)),
				iround(screen_origin_to_sprite_center_px_vec.y + (F(src_rect.h) * M2_GAME.Dimensions().OutputPixelsPerMeter() * 11.0f / F(sprite.Ppm()) / 2.0f / 10.0f)), // Give an offset of 1.1
				iround(M2_GAME.Dimensions().OutputPixelsPerMeter()),
				iround(M2_GAME.Dimensions().OutputPixelsPerMeter() * 12.0f / 100.0f) // 0.15 m height
		};
		} else {
			const auto obj_position = gfx.owner().position;
			// Place add-on below the sprite
			const auto addon_position = m3::VecF{obj_position.x, obj_position.y, -0.2f};
			if (const auto projected_addon_position = screen_origin_to_projection_along_camera_plane_dstpx(addon_position)) {
				const auto rect = RectI::centered_around(VecI{*projected_addon_position}, iround(M2_GAME.Dimensions().OutputPixelsPerMeter()), iround(M2_GAME.Dimensions().OutputPixelsPerMeter() * 12.0f / 100.0f));
				dst_rect = static_cast<SDL_Rect>(rect);
			}
		}

		// Black background
		SDL_SetRenderDrawColor(M2_GAME.renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(M2_GAME.renderer, &dst_rect);

		// Green part
		const float percentage = (*gfx.draw_addon_health_bar) < 0.0f ? 0.0f : (1.0f < *gfx.draw_addon_health_bar) ? 1.0f : *gfx.draw_addon_health_bar;
		const auto green_rect = SDL_Rect{dst_rect.x + 1, dst_rect.y + 1, I(roundf(percentage * F(dst_rect.w - 2))), dst_rect.h - 2};
		SDL_SetRenderDrawColor(M2_GAME.renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(M2_GAME.renderer, &green_rect);
	} else {
		// This function only works if there is a sprite
	}
}

void m2::Graphic::color_cell(const VecI& cell, SDL_Color color) {
	const auto screen_origin_to_cell_center_px = ScreenOriginToPositionVecPx(VecF{cell});
	const auto rect = SDL_Rect{
		iround(screen_origin_to_cell_center_px.x - (M2_GAME.Dimensions().OutputPixelsPerMeter() / 2.0f)),
		iround(screen_origin_to_cell_center_px.y - (M2_GAME.Dimensions().OutputPixelsPerMeter() / 2.0f)),
		iround(M2_GAME.Dimensions().OutputPixelsPerMeter()),
		iround(M2_GAME.Dimensions().OutputPixelsPerMeter())
	};

	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	SDL_RenderFillRect(M2_GAME.renderer, &rect);
}

void m2::Graphic::color_rect(const RectF& world_coordinates_m, SDL_Color color) {
	const auto screen_origin_to_top_left_px = ScreenOriginToPositionVecPx(world_coordinates_m.top_left());
	const auto screen_origin_to_bottom_right_px = ScreenOriginToPositionVecPx(world_coordinates_m.bottom_right());
	const auto rect = SDL_Rect{
			(int)roundf(screen_origin_to_top_left_px.x),
			(int)roundf(screen_origin_to_top_left_px.y),
			(int)roundf(screen_origin_to_bottom_right_px.x - screen_origin_to_top_left_px.x),
			(int)roundf(screen_origin_to_bottom_right_px.y - screen_origin_to_top_left_px.y)
	};

	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	SDL_RenderFillRect(M2_GAME.renderer, &rect);
}
void m2::Graphic::color_rect(const RectF& world_coordinates_m, const RGB& color) {
	color_rect(world_coordinates_m, SDL_Color{color.r, color.g, color.b, 255});
}

void m2::Graphic::color_disk(const VecF& center_position_m, float radius_m, const SDL_Color& color) {
	const auto center_position_px = ScreenOriginToPositionVecPx(center_position_m);
	const auto radius_px = radius_m * M2_GAME.Dimensions().OutputPixelsPerMeter();
	sdl::draw_disk(M2_GAME.renderer, center_position_px, color, radius_px, color);
}

void m2::Graphic::draw_cross(const VecF& world_position, SDL_Color color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	const auto draw_position = VecI{ScreenOriginToPositionVecPx(world_position)};
	SDL_RenderDrawLine(M2_GAME.renderer, draw_position.x - 9, draw_position.y - 9, draw_position.x + 10, draw_position.y + 10);
	SDL_RenderDrawLine(M2_GAME.renderer, draw_position.x - 9, draw_position.y + 9, draw_position.x + 10, draw_position.y - 10);
}

void m2::Graphic::draw_line(const VecF& world_position_1, const VecF& world_position_2, SDL_Color color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	if (is_projection_type_parallel(M2_LEVEL.ProjectionType())) {
		const auto p1 = static_cast<VecI>(ScreenOriginToPositionVecPx(world_position_1));
		const auto p2 = static_cast<VecI>(ScreenOriginToPositionVecPx(world_position_2));
		SDL_RenderDrawLine(M2_GAME.renderer, p1.x, p1.y, p2.x, p2.y);
	} else {
		const auto p1 = m3::screen_origin_to_projection_along_camera_plane_dstpx(m3::VecF{world_position_1});
		const auto p2 = m3::screen_origin_to_projection_along_camera_plane_dstpx(m3::VecF{world_position_2});
		if (p1 && p2) {
			SDL_RenderDrawLineF(M2_GAME.renderer, p1->x, p1->y, p2->x, p2->y);
		}
	}
}

void m2::Graphic::draw_vertical_line(float x, SDL_Color color) {
	const auto x_px = static_cast<int>(roundf(ScreenOriginToPositionVecPx(VecF{x, 0.0f}).x));
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(M2_GAME.renderer, x_px, M2_GAME.Dimensions().Game().y, x_px, M2_GAME.Dimensions().Game().y + M2_GAME.Dimensions().Game().h);
}

void m2::Graphic::draw_horizontal_line(float y, SDL_Color color) {
	const auto y_px = static_cast<int>(roundf(ScreenOriginToPositionVecPx(VecF{0.0f, y}).y));
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(M2_GAME.renderer, M2_GAME.Dimensions().Game().x, y_px, M2_GAME.Dimensions().Game().x + M2_GAME.Dimensions().Game().w, y_px);
}

bool m2::Graphic::dim_rendering_if_necessary(Id object_id, SDL_Texture* texture) {
	// Dim the sprite if dimming mode is enabled
	if (const auto& DimmingExceptions = M2_LEVEL.DimmingExceptions()) {
		if (not DimmingExceptions->contains(object_id)) {
			static uint8_t mod = uround(M2G_PROXY.dimming_factor * F(255));
			SDL_SetTextureColorMod(texture, mod, mod, mod);
			return true;
		}
	}
	return false;
}

void m2::Graphic::undim_rendering(SDL_Texture* texture) {
	SDL_SetTextureColorMod(texture, 255, 255, 255);
}

void m2::DrawTextureIn2dWorld(
		SDL_Renderer* renderer,
		SDL_Texture* sourceTexture,
		const SDL_Rect* sourceRect,
		const float originalRotationOfSourceTextureInRadians,
		const float outputToSourcePpmRatio,
		const VecF& textureCenterToTextureOriginVecInOutputPixels,
		const VecF& screenOriginToTextureCenterVecInOutputPixels,
		const float rotationToApplyInRadians) {

	// Calculate the destination
	const auto dstRect = SDL_Rect{
		I(screenOriginToTextureCenterVecInOutputPixels.x - F(sourceRect->w) * outputToSourcePpmRatio / 2.0f),
		I(screenOriginToTextureCenterVecInOutputPixels.y - F(sourceRect->h) * outputToSourcePpmRatio / 2.0f),
		iceil(F(sourceRect->w) * outputToSourcePpmRatio),
		iceil(F(sourceRect->h) * outputToSourcePpmRatio)
		// TODO using I() and ceilf() here is quite problematic, but I couldn't find any other way of ensuring not
		//  leaving any gaps between sprites
		// TODO unfortunately, we can't draw pixel perfect sprites with floating point scaling. However, the game can
		//  avoid flickering by avoiding highly repeating patterns.
	};

	// Calculate the center point used for rotation origin
	const auto centerPoint = SDL_Point{
		iround(textureCenterToTextureOriginVecInOutputPixels.x) + dstRect.w / 2,
		iround(textureCenterToTextureOriginVecInOutputPixels.y) + dstRect.h / 2
	};

	// Render
	const auto renderModeResult = SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	m2_expect_zero_or_throw_message(renderModeResult, SDL_GetError());
	const auto renderResult = SDL_RenderCopyEx(renderer, sourceTexture, sourceRect, &dstRect,
			to_degrees(rotationToApplyInRadians - originalRotationOfSourceTextureInRadians), &centerPoint, SDL_FLIP_NONE);
	m2_expect_zero_or_throw_message(renderResult, SDL_GetError());
}
void m2::DrawTextureIn3dWorld(
		SDL_Renderer* renderer,
		SDL_Texture* sourceTexture,
		const SDL_Rect* sourceRect,
		const float sourcePpm,
		const VecF& sourceCenterToOriginVectorInOutputPixels,
		const float originalRotationOfSourceTextureInRadians,
		const VecF& sourceTextureSheetDimensions,
		const VecF& xyPositionInWorldM,
		const float zPositionInWorldM,
		const float rotationToApplyInRadians,
		const bool isForeground) {
	// Draw two triangles in one call
	// 0****1
	// *   **
	// *  * *
	// * *  *
	// **   *
	// 2****3

	auto position3 = m3::VecF{xyPositionInWorldM.x, xyPositionInWorldM.y, 0.0f};
	m3::VecF point_0, point_1, point_2, point_3;
	if (isForeground) {
		auto sprite_x_offset_in_dest_px = sourceCenterToOriginVectorInOutputPixels.x;
		auto point_0_not_rotated = m3::VecF{
				xyPositionInWorldM.x - F(sourceRect->w) / sourcePpm / 2.0f - sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter(),
				xyPositionInWorldM.y,
				F(sourceRect->h) / sourcePpm
		};
		auto point_1_not_rotated = m3::VecF{
				xyPositionInWorldM.x + ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.y,
				(float)sourceRect->h / sourcePpm
		};
		auto point_2_not_rotated = m3::VecF{
				xyPositionInWorldM.x - ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.y,
				0.0f
		};
		auto point_3_not_rotated = m3::VecF{
				xyPositionInWorldM.x + ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.y,
				0.0f
		};

		auto position_to_point_0_not_rotated = point_0_not_rotated - position3;
		auto position_to_point_1_not_rotated = point_1_not_rotated - position3;
		auto position_to_point_2_not_rotated = point_2_not_rotated - position3;
		auto position_to_point_3_not_rotated = point_3_not_rotated - position3;

		// Apply sprite and object rotation
		auto xz_rotation = originalRotationOfSourceTextureInRadians;
		auto xy_rotation = rotationToApplyInRadians;
		auto position_to_point_0 = position_to_point_0_not_rotated.rotate_xz(xz_rotation).rotate_xy(xy_rotation);
		auto position_to_point_1 = position_to_point_1_not_rotated.rotate_xz(xz_rotation).rotate_xy(xy_rotation);
		auto position_to_point_2 = position_to_point_2_not_rotated.rotate_xz(xz_rotation).rotate_xy(xy_rotation);
		auto position_to_point_3 = position_to_point_3_not_rotated.rotate_xz(xz_rotation).rotate_xy(xy_rotation);

		point_0 = (position_to_point_0 + position3).offset_z(zPositionInWorldM);
		point_1 = (position_to_point_1 + position3).offset_z(zPositionInWorldM);
		point_2 = (position_to_point_2 + position3).offset_z(zPositionInWorldM);
		point_3 = (position_to_point_3 + position3).offset_z(zPositionInWorldM);
	} else {
		// Background sprite
		auto sprite_x_offset_in_dest_px = sourceCenterToOriginVectorInOutputPixels.x;
		auto sprite_y_offset_in_dest_px = sourceCenterToOriginVectorInOutputPixels.y;
		auto point_0_not_rotated = m3::VecF{
				xyPositionInWorldM.x - ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.y - ((float)sourceRect->h / sourcePpm / 2.0f) - (sprite_y_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				0.0f
		};
		auto point_1_not_rotated = m3::VecF{
				xyPositionInWorldM.x + ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.y - ((float)sourceRect->h / sourcePpm / 2.0f) - (sprite_y_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				0.0f
		};
		auto point_2_not_rotated = m3::VecF{
				xyPositionInWorldM.x - ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.y + ((float)sourceRect->h / sourcePpm / 2.0f) - (sprite_y_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				0.0f
		};
		auto point_3_not_rotated = m3::VecF{
				xyPositionInWorldM.x + ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.y + ((float)sourceRect->h / sourcePpm / 2.0f) - (sprite_y_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				0.0f
		};

		auto position_to_point_0_not_rotated = point_0_not_rotated - position3;
		auto position_to_point_1_not_rotated = point_1_not_rotated - position3;
		auto position_to_point_2_not_rotated = point_2_not_rotated - position3;
		auto position_to_point_3_not_rotated = point_3_not_rotated - position3;

		// Apply sprite and object rotation
		auto xy_rotation = rotationToApplyInRadians - originalRotationOfSourceTextureInRadians;
		auto position_to_point_0 = position_to_point_0_not_rotated.rotate_xy(xy_rotation);
		auto position_to_point_1 = position_to_point_1_not_rotated.rotate_xy(xy_rotation);
		auto position_to_point_2 = position_to_point_2_not_rotated.rotate_xy(xy_rotation);
		auto position_to_point_3 = position_to_point_3_not_rotated.rotate_xy(xy_rotation);

		point_0 = (position_to_point_0 + position3).offset_z(zPositionInWorldM);
		point_1 = (position_to_point_1 + position3).offset_z(zPositionInWorldM);
		point_2 = (position_to_point_2 + position3).offset_z(zPositionInWorldM);
		point_3 = (position_to_point_3 + position3).offset_z(zPositionInWorldM);
	}

	const auto projected_point_0 = m3::screen_origin_to_projection_along_camera_plane_dstpx(point_0);
	const auto projected_point_1 = m3::screen_origin_to_projection_along_camera_plane_dstpx(point_1);
	const auto projected_point_2 = m3::screen_origin_to_projection_along_camera_plane_dstpx(point_2);
	const auto projected_point_3 = m3::screen_origin_to_projection_along_camera_plane_dstpx(point_3);

	if (projected_point_0 && projected_point_1 && projected_point_2 && projected_point_3) {
		SDL_Vertex vertices[4] = {};
		vertices[0].position = static_cast<SDL_FPoint>(*projected_point_0);
		vertices[0].color = {255, 255, 255, 255};
		vertices[0].tex_coord = SDL_FPoint{
				F(sourceRect->x) / sourceTextureSheetDimensions.x,
				F(sourceRect->y) / sourceTextureSheetDimensions.y,
		};

		vertices[1].position = static_cast<SDL_FPoint>(*projected_point_1);
		vertices[1].color = {255, 255, 255, 255};
		vertices[1].tex_coord = SDL_FPoint{
				F(sourceRect->x + sourceRect->w) / sourceTextureSheetDimensions.x,
				F(sourceRect->y) / sourceTextureSheetDimensions.y,
		};

		vertices[2].position = static_cast<SDL_FPoint>(*projected_point_2);
		vertices[2].color = {255, 255, 255, 255};
		vertices[2].tex_coord = SDL_FPoint{
				F(sourceRect->x) / sourceTextureSheetDimensions.x,
				F(sourceRect->y + sourceRect->h) / sourceTextureSheetDimensions.y,
		};

		vertices[3].position = static_cast<SDL_FPoint>(*projected_point_3);
		vertices[3].color = {255, 255, 255, 255};
		vertices[3].tex_coord = SDL_FPoint{
				F(sourceRect->x + sourceRect->w) / sourceTextureSheetDimensions.x,
				F(sourceRect->y + sourceRect->h) / sourceTextureSheetDimensions.y,
		};

		static const int indices[6] = {0, 1, 2, 2, 1, 3};

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderGeometry(renderer, sourceTexture, vertices, 4, indices, 6);
	}
}
