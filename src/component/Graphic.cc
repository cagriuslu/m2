#include <m2/m3/VecF.h>
#include <m2/Component.h>
#include <m2/Game.h>
#include "m2/component/Graphic.h"
#include <m2/Object.h>
#include <cmath>

bool m2::IsProjectionTypeParallel(const pb::ProjectionType pt) {
	return pt == pb::PARALLEL;
}
bool m2::IsProjectionTypePerspective(const pb::ProjectionType pt) {
	return pt == pb::PERSPECTIVE_YZ;
}

m2::VecF m2::CameraToPositionVecM(const VecF& position) {
	const auto* camera = M2_LEVEL.objects.Get(M2_LEVEL.cameraId);
	return position - static_cast<VecF>(camera->GetPhysique().position);
}
m2::VecF m2::CameraToPositionVecPx(const VecF& position) {
	// Find the vector from camera to position and multiply with output PPM to convert into output pixels
	return CameraToPositionVecM(position) * M2_GAME.Dimensions().OutputPixelsPerMeter();
}
m2::VecF m2::ScreenOriginToPositionVecPx(const VecF& position) {
	return CameraToPositionVecPx(position) + VecF{M2_GAME.Dimensions().WindowDimensions().x / 2, M2_GAME.Dimensions().WindowDimensions().y / 2 };
}
m2::VecF m2::PixelToPositionVecM(const VecI& pixelPosition) {
	if (M2_LEVEL.GetProjectionType() != pb::PARALLEL) {
		throw M2_ERROR("Unable to calculate pixel position for non-parallel projection");
	}
	const auto screenCenterToPixelPositionVectorInPixels =
			VecI{pixelPosition.x - (M2_GAME.Dimensions().WindowDimensions().x / 2),
				pixelPosition.y - (M2_GAME.Dimensions().WindowDimensions().y / 2)};
	const auto screenCenterToPixelPositionVectorInMeters =
			VecF{ToFloat(screenCenterToPixelPositionVectorInPixels.x) / M2_GAME.Dimensions().OutputPixelsPerMeter(),
				ToFloat(screenCenterToPixelPositionVectorInPixels.y) / M2_GAME.Dimensions().OutputPixelsPerMeter()};
	const auto camera_position = M2_LEVEL.objects[M2_LEVEL.cameraId].GetPhysique().position;
	return screenCenterToPixelPositionVectorInMeters + static_cast<VecF>(camera_position);
}
m2::RectF m2::ViewportM() {
	if (M2_LEVEL.GetProjectionType() != pb::PARALLEL) {
		throw M2_ERROR("Unable to calculate viewport for non-parallel projection");
	}
	const auto top_left = PixelToPositionVecM(VecI{M2_GAME.Dimensions().Game().x, M2_GAME.Dimensions().Game().y});
	const auto bottom_right = PixelToPositionVecM(VecI{M2_GAME.Dimensions().Game().GetX2(), M2_GAME.Dimensions().Game().GetY2()});
	return RectF::CreateFromCorners(top_left, bottom_right);
}

m3::VecF m3::CameraPositionM() {
	const auto* camera = M2_LEVEL.objects.Get(M2_LEVEL.cameraId);
	const auto& cameraPosition2d = camera->InferPositionF();
	const auto raw_camera_position = VecF{cameraPosition2d.GetX(), cameraPosition2d.GetY(), 0.0f};
	const auto camera_position = raw_camera_position + M2_LEVEL.GetCameraOffset();
	return camera_position;
}
m3::VecF m3::FocusPositionM() {
	const auto playerPosition = M2_PLAYER.InferPositionF();
	return {playerPosition.GetX(), playerPosition.GetY(), M2G_PROXY.focus_point_height};
}
float m3::VisibleWidthM() {
	// Do not recalculate unless the distance or FOV has changed
	static m3::VecF prev_camera_offset;
	static auto prev_horizontal_fov = INFINITY;

	static float visible_width{};
	if (const auto camera_offset = M2_LEVEL.GetCameraOffset(); prev_camera_offset != camera_offset || prev_horizontal_fov != M2_LEVEL.GetHorizontalFov()) {
		prev_camera_offset = camera_offset;
		prev_horizontal_fov = M2_LEVEL.GetHorizontalFov();

		const auto tan_of_half_horizontal_fov = tanf(m2::ToRadians(M2_LEVEL.GetHorizontalFov()) / 2.0f);
		visible_width = 2 * camera_offset.length() * tan_of_half_horizontal_fov;
	}
	return visible_width;
}
float m3::Ppm() {
	return static_cast<float>(M2_GAME.Dimensions().Game().w) / VisibleWidthM();
}

m3::Line m3::CameraToPositionLine(const VecF& position) {
	return Line::from_points(CameraPositionM(), position);
}
m3::Plane m3::FocusToCameraPlane() {
	const auto focus_position = FocusPositionM();
	const auto normal = CameraPositionM() - focus_position;
	return Plane{normal, focus_position};
}
std::optional<m3::VecF> m3::Projection(const VecF& position) {
	auto [intersection_point, forward_intersection] = FocusToCameraPlane()
			.intersection(CameraToPositionLine(position));
	if (not forward_intersection) {
		return {};
	}
	return intersection_point;
}
std::optional<m3::VecF> m3::FocusToProjectionM(const VecF& position) {
	const auto proj = Projection(position);
	if (not proj) {
		return std::nullopt;
	}
	return *proj - FocusPositionM();
}
std::optional<m2::VecF> m3::FocusToProjectionInCameraPlaneCoordinatesM(const VecF& position) {
	const auto focus_to_projection = FocusToProjectionM(position);
	if (not focus_to_projection) {
		return {};
	}

	float horizontal_projection, vertical_projection;
	if (M2_LEVEL.GetProjectionType() == m2::pb::PERSPECTIVE_YZ) {
		static const auto unit_vector_along_x = VecF{1.0f, 0.0f, 0.0f};
		horizontal_projection = focus_to_projection->dot(unit_vector_along_x);
		const auto projection_x = VecF{horizontal_projection, 0.0f, 0.0f};

		// y-axis of the projection plane is not along the real y-axis
		// Use dot product, or better, find the length and the sign of it
		const auto projection_y = *focus_to_projection - projection_x;
		const auto projection_y_length = projection_y.length();
		const auto projection_y_sign = 0 <= projection_y.y ? 1.0f : -1.0f;
		vertical_projection = projection_y_length * projection_y_sign;
	} else {
		throw M2_ERROR("Invalid ProjectionType");
	}
	return m2::VecF{horizontal_projection, vertical_projection};
}
std::optional<m2::VecF> m3::FocusToProjectionInCameraPlaneCoordinatesDstpx(const VecF& position) {
	const auto focus_to_projection_along_camera_plane = FocusToProjectionInCameraPlaneCoordinatesM(position);
	if (not focus_to_projection_along_camera_plane) {
		return {};
	}
	const auto pixels_per_meter = Ppm();
	return *focus_to_projection_along_camera_plane * pixels_per_meter;
}
std::optional<m2::VecF> m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(const VecF& position) {
	const auto focus_to_projection_along_camera_plane_px = FocusToProjectionInCameraPlaneCoordinatesDstpx(position);
	if (not focus_to_projection_along_camera_plane_px) {
		return {};
	}
	return *focus_to_projection_along_camera_plane_px + m2::VecF{M2_GAME.Dimensions().WindowDimensions().x / 2, M2_GAME.Dimensions().WindowDimensions().y / 2 };
}

m2::Graphic::Graphic(const Id ownerId, const VecF& position) : Component(ownerId), position(position) {}
m2::Graphic::Graphic(const uint64_t object_id, const std::variant<Sprite, pb::TextLabel>& spriteOrTextLabel, const VecF& position)
		: Component(object_id), position(position), onDraw(DefaultDrawCallback) {
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
		const bool is_foreground = M2_LEVEL.uprightGraphics.GetId(&gfx);
		const auto projector = IsProjectionTypePerspective(M2_LEVEL.GetProjectionType())
				? &Sprite::DrawIn3dWorld
				: &Sprite::DrawIn2dWorld;
		const auto spriteDrawer = [&](const bool foregroundCompanion) -> void {
			(sprite.*projector)(gfx.position, foregroundCompanion, gfx.orientation, is_foreground, gfx.z);
		};

		// Dim the sprite if dimming mode is enabled. TODO Dimming is implemented only for default variant.
		const bool dimmed = DimRenderingIfNecessary(gfx.OwnerId(), sprite.GetTexture());

		spriteDrawer(gfx.drawForegroundCompanion);

		// If dimming was active, we need to un-dim.
		if (dimmed) {
			UndimRendering(sprite.GetTexture());
		}
	} else if (std::holds_alternative<const pb::TextLabel*>(gfx.visual)) {
		const auto& textLabel = *std::get<const pb::TextLabel*>(gfx.visual);

		// Generate the text label if necessary
		if (not gfx.textLabelRect) {
			// Font size is the same as text height in output pixels
			gfx.textLabelRect = M2_GAME.GetTextLabelCache().Create(textLabel.text(), FontSizeOfTextLabel(textLabel));
		}

		// Dim the sprite if dimming mode is enabled.
		const bool dimmed = DimRenderingIfNecessary(gfx.OwnerId(), M2_GAME.GetTextLabelCache().Texture());

		// Draw background
		if (textLabel.background_color().a()) {
			DrawTextLabelBackgroundIn2dWorld(textLabel, gfx.textLabelRect, gfx.position, dimmed);
		}
		// Draw text label
		const bool is_foreground = M2_LEVEL.uprightGraphics.GetId(&gfx);
		const auto projector = IsProjectionTypePerspective(M2_LEVEL.GetProjectionType())
				? &DrawTextLabelIn3dWorld
				: &DrawTextLabelIn2dWorld;
		projector(textLabel, gfx.textLabelRect, gfx.position, gfx.orientation, is_foreground, gfx.z);

		// If dimming was active, we need to un-dim.
		if (dimmed) {
			UndimRendering(M2_GAME.GetTextLabelCache().Texture());
		}
	} else {
		// This function only draws visuals
	}
}

void m2::Graphic::ColorCell(const VecI& cell, const SDL_Color color) {
	const auto screen_origin_to_cell_center_px = ScreenOriginToPositionVecPx(VecF{cell});
	const auto rect = SDL_Rect{
		RoundI(screen_origin_to_cell_center_px.GetX() - (M2_GAME.Dimensions().OutputPixelsPerMeter() / 2.0f)),
		RoundI(screen_origin_to_cell_center_px.GetY() - (M2_GAME.Dimensions().OutputPixelsPerMeter() / 2.0f)),
		RoundI(M2_GAME.Dimensions().OutputPixelsPerMeter()),
		RoundI(M2_GAME.Dimensions().OutputPixelsPerMeter())
	};

	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	SDL_RenderFillRect(M2_GAME.renderer, &rect);
}
void m2::Graphic::ColorRect(const RectF& world_coordinates_m, const SDL_Color color) {
	const auto screen_origin_to_top_left_px = ScreenOriginToPositionVecPx(world_coordinates_m.GetTopLeftPoint());
	const auto screen_origin_to_bottom_right_px = ScreenOriginToPositionVecPx(world_coordinates_m.GetBottomRightPoint());
	const auto rect = SDL_Rect{
			I(screen_origin_to_top_left_px.GetX()),
			I(screen_origin_to_top_left_px.GetY()),
			CeilI(screen_origin_to_bottom_right_px.GetX() - screen_origin_to_top_left_px.GetX()),
			CeilI(screen_origin_to_bottom_right_px.GetY() - screen_origin_to_top_left_px.GetY())
			// TODO using I() and ceilf() here is quite problematic, but I couldn't find any other way of ensuring not
			//  leaving any gaps between sprites
			// TODO unfortunately, we can't draw pixel perfect sprites with floating point scaling. However, the game can
			//  avoid flickering by avoiding highly repeating patterns.
	};

	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	SDL_RenderFillRect(M2_GAME.renderer, &rect);
}
void m2::Graphic::ColorRect(const RectF& world_coordinates_m, const RGB& color) {
	ColorRect(world_coordinates_m, SDL_Color{color.r, color.g, color.b, 255});
}
void m2::Graphic::ColorRect(const RectF& world_coordinates_m, const RGBA& color) {
	ColorRect(world_coordinates_m, SDL_Color{color.r, color.g, color.b, color.a});
}
void m2::Graphic::ColorDisk(const VecF& center_position_m, const float radius_m, const SDL_Color& color) {
	const auto center_position_px = ScreenOriginToPositionVecPx(center_position_m);
	const auto radius_px = radius_m * M2_GAME.Dimensions().OutputPixelsPerMeter();
	sdl::draw_disk(M2_GAME.renderer, center_position_px, color, radius_px, color);
}
void m2::Graphic::DrawCross(const VecF& world_position, SDL_Color color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	const auto draw_position = VecI{ScreenOriginToPositionVecPx(world_position)};
	SDL_RenderDrawLine(M2_GAME.renderer, draw_position.x - 9, draw_position.y - 9, draw_position.x + 10, draw_position.y + 10);
	SDL_RenderDrawLine(M2_GAME.renderer, draw_position.x - 9, draw_position.y + 9, draw_position.x + 10, draw_position.y - 10);
}
void m2::Graphic::DrawCross(const VecF& worldPosition, int radiusPx, const RGBA& color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	const auto draw_position = VecI{ScreenOriginToPositionVecPx(worldPosition)};
	SDL_RenderDrawLine(M2_GAME.renderer, draw_position.x - radiusPx, draw_position.y - radiusPx, draw_position.x + radiusPx, draw_position.y + radiusPx);
	SDL_RenderDrawLine(M2_GAME.renderer, draw_position.x - radiusPx, draw_position.y + radiusPx, draw_position.x + radiusPx, draw_position.y - radiusPx);
}
void m2::Graphic::DrawCross(const VecF& worldPosition, const float radiusM, const RGBA& color) {
	DrawLine(worldPosition + VecF{-radiusM, -radiusM}, worldPosition + VecF{radiusM, radiusM}, color);
	DrawLine(worldPosition + VecF{-radiusM, radiusM}, worldPosition + VecF{radiusM, -radiusM}, color);
}
void m2::Graphic::DrawLine(const VecF& world_position_1, const VecF& world_position_2, SDL_Color color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	if (IsProjectionTypeParallel(M2_LEVEL.GetProjectionType())) {
		const auto p1 = static_cast<VecI>(ScreenOriginToPositionVecPx(world_position_1));
		const auto p2 = static_cast<VecI>(ScreenOriginToPositionVecPx(world_position_2));
		SDL_RenderDrawLine(M2_GAME.renderer, p1.x, p1.y, p2.x, p2.y);
	} else {
		const auto p1 = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(m3::VecF{world_position_1});
		const auto p2 = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(m3::VecF{world_position_2});
		if (p1 && p2) {
			SDL_RenderDrawLineF(M2_GAME.renderer, p1->GetX(), p1->GetY(), p2->GetX(), p2->GetY());
		}
	}
}
void m2::Graphic::DrawLine(const VecF& worldPosition1M, const VecF& worldPosition2M, const RGBA& color) {
	DrawLine(worldPosition1M, worldPosition2M, SDL_Color{color.r, color.g, color.b, color.a});
}
void m2::Graphic::DrawVerticalLine(float x, const RGBA& color) {
	const auto x_px = static_cast<int>(roundf(ScreenOriginToPositionVecPx(VecF{x, 0.0f}).GetX()));
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(M2_GAME.renderer, x_px, M2_GAME.Dimensions().Game().y, x_px, M2_GAME.Dimensions().Game().y + M2_GAME.Dimensions().Game().h);
}
void m2::Graphic::DrawHorizontalLine(float y, const RGBA& color) {
	const auto y_px = static_cast<int>(roundf(ScreenOriginToPositionVecPx(VecF{0.0f, y}).GetY()));
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(M2_GAME.renderer, M2_GAME.Dimensions().Game().x, y_px, M2_GAME.Dimensions().Game().x + M2_GAME.Dimensions().Game().w, y_px);
}
void m2::Graphic::DrawRectangle(const VecF& center, float width, float height, float orientationRads, const RGBA& color) {
	const auto topLeft = center + VecF{-width / 2.0f, -height / 2.0f}.Rotate(orientationRads);
	const auto topRight = center + VecF{width / 2.0f, -height / 2.0f}.Rotate(orientationRads);
	const auto bottomLeft = center + VecF{-width / 2.0f, height / 2.0f}.Rotate(orientationRads);
	const auto bottomRight = center + VecF{width / 2.0f, height / 2.0f}.Rotate(orientationRads);
	DrawLine(topLeft, topRight, color);
	DrawLine(topRight, bottomRight, color);
	DrawLine(bottomRight, bottomLeft, color);
	DrawLine(bottomLeft, topLeft, color);
}
void m2::Graphic::DrawGridLines(const float startFrom, const float frequency, const RGBA& color) {
	const auto viewport = ViewportM();
	// Remove the offset
	const auto viewportNoOffset = viewport.Shift({-startFrom, -startFrom});
	// Divide by frequency
	const auto multiple = VecF{std::ceil(viewportNoOffset.x / frequency), std::floor(viewportNoOffset.y / frequency)};
	for (auto x = multiple.GetX() * frequency + startFrom; x <= viewport.GetX2(); x += frequency) {
		DrawVerticalLine(x, color);
	}
	for (auto y = multiple.GetY() * frequency + startFrom; y <= viewport.GetY2(); y += frequency) {
		DrawHorizontalLine(y, color);
	}
}

bool m2::Graphic::DimRenderingIfNecessary(Id object_id, SDL_Texture* texture) {
	// Dim the sprite if dimming mode is enabled
	if (const auto& DimmingExceptions = M2_LEVEL.GetDimmingExceptions()) {
		if (not DimmingExceptions->contains(object_id)) {
			static uint8_t mod = static_cast<uint8_t>(RoundU(M2G_PROXY.dimming_factor * ToFloat(255)));
			SDL_SetTextureColorMod(texture, mod, mod, mod);
			return true;
		}
	}
	return false;
}

void m2::Graphic::UndimRendering(SDL_Texture* texture) {
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
		I(screenOriginToTextureCenterVecInOutputPixels.GetX() - ToFloat(sourceRect->w) * outputToSourcePpmRatio / 2.0f),
		I(screenOriginToTextureCenterVecInOutputPixels.GetY() - ToFloat(sourceRect->h) * outputToSourcePpmRatio / 2.0f),
		CeilI(ToFloat(sourceRect->w) * outputToSourcePpmRatio),
		CeilI(ToFloat(sourceRect->h) * outputToSourcePpmRatio)
		// TODO using I() and ceilf() here is quite problematic, but I couldn't find any other way of ensuring not
		//  leaving any gaps between sprites
		// TODO unfortunately, we can't draw pixel perfect sprites with floating point scaling. However, the game can
		//  avoid flickering by avoiding highly repeating patterns.
	};

	// Calculate the center point used for rotation origin
	const auto centerPoint = SDL_Point{
		RoundI(textureCenterToTextureOriginVecInOutputPixels.GetX()) + dstRect.w / 2,
		RoundI(textureCenterToTextureOriginVecInOutputPixels.GetY()) + dstRect.h / 2
	};

	// Render
	const auto renderModeResult = SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	m2ExpectZeroOrThrowMessage(renderModeResult, SDL_GetError());
	const auto renderResult = SDL_RenderCopyEx(renderer, sourceTexture, sourceRect, &dstRect,
			ToDegrees(rotationToApplyInRadians - originalRotationOfSourceTextureInRadians), &centerPoint, SDL_FLIP_NONE);
	m2ExpectZeroOrThrowMessage(renderResult, SDL_GetError());
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

	auto position3 = m3::VecF{xyPositionInWorldM.GetX(), xyPositionInWorldM.GetY(), 0.0f};
	m3::VecF point_0, point_1, point_2, point_3;
	if (isForeground) {
		auto sprite_x_offset_in_dest_px = sourceCenterToOriginVectorInOutputPixels.GetX();
		auto point_0_not_rotated = m3::VecF{
				xyPositionInWorldM.GetX() - ToFloat(sourceRect->w) / sourcePpm / 2.0f - sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter(),
				xyPositionInWorldM.GetY(),
				ToFloat(sourceRect->h) / sourcePpm
		};
		auto point_1_not_rotated = m3::VecF{
				xyPositionInWorldM.GetX() + ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.GetY(),
				(float)sourceRect->h / sourcePpm
		};
		auto point_2_not_rotated = m3::VecF{
				xyPositionInWorldM.GetX() - ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.GetY(),
				0.0f
		};
		auto point_3_not_rotated = m3::VecF{
				xyPositionInWorldM.GetX() + ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.GetY(),
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
		auto sprite_x_offset_in_dest_px = sourceCenterToOriginVectorInOutputPixels.GetX();
		auto sprite_y_offset_in_dest_px = sourceCenterToOriginVectorInOutputPixels.GetY();
		auto point_0_not_rotated = m3::VecF{
				xyPositionInWorldM.GetX() - ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.GetY() - ((float)sourceRect->h / sourcePpm / 2.0f) - (sprite_y_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				0.0f
		};
		auto point_1_not_rotated = m3::VecF{
				xyPositionInWorldM.GetX() + ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.GetY() - ((float)sourceRect->h / sourcePpm / 2.0f) - (sprite_y_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				0.0f
		};
		auto point_2_not_rotated = m3::VecF{
				xyPositionInWorldM.GetX() - ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.GetY() + ((float)sourceRect->h / sourcePpm / 2.0f) - (sprite_y_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				0.0f
		};
		auto point_3_not_rotated = m3::VecF{
				xyPositionInWorldM.GetX() + ((float)sourceRect->w / sourcePpm / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
				xyPositionInWorldM.GetY() + ((float)sourceRect->h / sourcePpm / 2.0f) - (sprite_y_offset_in_dest_px / M2_GAME.Dimensions().OutputPixelsPerMeter()),
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

	const auto projected_point_0 = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(point_0);
	const auto projected_point_1 = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(point_1);
	const auto projected_point_2 = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(point_2);
	const auto projected_point_3 = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(point_3);

	if (projected_point_0 && projected_point_1 && projected_point_2 && projected_point_3) {
		SDL_Vertex vertices[4] = {};
		vertices[0].position = static_cast<SDL_FPoint>(*projected_point_0);
		vertices[0].color = {255, 255, 255, 255};
		vertices[0].tex_coord = SDL_FPoint{
				ToFloat(sourceRect->x) / sourceTextureSheetDimensions.GetX(),
				ToFloat(sourceRect->y) / sourceTextureSheetDimensions.GetY(),
		};

		vertices[1].position = static_cast<SDL_FPoint>(*projected_point_1);
		vertices[1].color = {255, 255, 255, 255};
		vertices[1].tex_coord = SDL_FPoint{
				ToFloat(sourceRect->x + sourceRect->w) / sourceTextureSheetDimensions.GetX(),
				ToFloat(sourceRect->y) / sourceTextureSheetDimensions.GetY(),
		};

		vertices[2].position = static_cast<SDL_FPoint>(*projected_point_2);
		vertices[2].color = {255, 255, 255, 255};
		vertices[2].tex_coord = SDL_FPoint{
				ToFloat(sourceRect->x) / sourceTextureSheetDimensions.GetX(),
				ToFloat(sourceRect->y + sourceRect->h) / sourceTextureSheetDimensions.GetY(),
		};

		vertices[3].position = static_cast<SDL_FPoint>(*projected_point_3);
		vertices[3].color = {255, 255, 255, 255};
		vertices[3].tex_coord = SDL_FPoint{
				ToFloat(sourceRect->x + sourceRect->w) / sourceTextureSheetDimensions.GetX(),
				ToFloat(sourceRect->y + sourceRect->h) / sourceTextureSheetDimensions.GetY(),
		};

		static const int indices[6] = {0, 1, 2, 2, 1, 3};

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderGeometry(renderer, sourceTexture, vertices, 4, indices, 6);
	}
}
